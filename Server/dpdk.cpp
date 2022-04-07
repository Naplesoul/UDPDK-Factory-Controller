#include "dpdk.h"

#include <ofp.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <inttypes.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <list>
#include <mutex>
#include <condition_variable>

#define MAX_WORKERS		4

/**
 * Parsed command line application arguments
 */
typedef struct {
	int core_count;
	int if_count;		/**< Number of interfaces to be used */
	char **if_names;	/**< Array of pointers to interface names */
	char *cli_file;
} appl_args_t;

typedef struct {
	struct ofp_sockaddr_in addr;
	std::string str;
} message_t;

/* helper funcs */
static void parse_args(int argc, char *argv[], appl_args_t *appl_args);
static void print_info(char *progname, appl_args_t *appl_args);
static void usage(char *progname);
static void ofp_start_udpserver_thread(odp_instance_t instance, int core_id);
static int poll_thread(void *);
static int send_thread(void *);
static void notify(union ofp_sigval sv);

static int listen_port;
static std::list<message_t> msg_queue;
std::mutex msg_queue_mtx;
std::condition_variable msg_queue_cv;

ofp_global_param_t app_init_params; /**< global OFP init parms */

static void (*recv_callback)(const struct ofp_sockaddr_in &addr, void *buf, int nbytes);

/** Get rid of path in filename - only for unix-type paths using '/' */
#define NO_PATH(file_name) (strrchr((file_name), '/') ? \
				strrchr((file_name), '/') + 1 : (file_name))

/** local hook
 *
 * @param pkt odp_packet_t
 * @param protocol int
 * @return int
 *
 */
static enum ofp_return_code fastpath_local_hook(odp_packet_t pkt, void *arg)
{
	int protocol = *(int *)arg;
	(void) pkt;
	(void) protocol;
	return OFP_PKT_CONTINUE;
}

/**
 * Parse and store the command line arguments
 *
 * @param argc       argument count
 * @param argv[]     argument vector
 * @param appl_args  Store application arguments here
 */
static void parse_args(int argc, char *argv[], appl_args_t *appl_args)
{
	int opt;
	int long_index;
	char *names, *str, *token, *save;
	size_t len;
	int i;
	static struct option longopts[] = {
		{"count", required_argument, NULL, 'c'},
		{"interface", required_argument, NULL, 'i'},	/* return 'i' */
		{"help", no_argument, NULL, 'h'},		/* return 'h' */
		{"cli-file", required_argument,
			NULL, 'f'},/* return 'f' */
		{NULL, 0, NULL, 0}
	};

	memset(appl_args, 0, sizeof(*appl_args));

	while (1) {
		opt = getopt_long(argc, argv, "+c:i:hf:",
				  longopts, &long_index);

		if (opt == -1)
			break;	/* No more options */

		switch (opt) {
		case 'c':
			appl_args->core_count = atoi(optarg);
			break;
			/* parse packet-io interface names */
		case 'i':
			len = strlen(optarg);
			if (len == 0) {
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			len += 1;	/* add room for '\0' */

			names = (char *)malloc(len);
			if (names == NULL) {
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}

			/* count the number of tokens separated by ',' */
			strcpy(names, optarg);
			for (str = names, i = 0;; str = NULL, i++) {
				token = strtok_r(str, ",", &save);
				if (token == NULL)
					break;
			}
			appl_args->if_count = i;

			if (appl_args->if_count == 0) {
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}

			/* allocate storage for the if names */
			appl_args->if_names = (char**)
				calloc(appl_args->if_count, sizeof(char *));

			/* store the if names (reset names string) */
			strcpy(names, optarg);
			for (str = names, i = 0;; str = NULL, i++) {
				token = strtok_r(str, ",", &save);
				if (token == NULL)
					break;
				appl_args->if_names[i] = token;
			}
			break;

		case 'h':
			usage(argv[0]);
			exit(EXIT_SUCCESS);
			break;

		case 'f':
			len = strlen(optarg);
			if (len == 0) {
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			len += 1;	/* add room for '\0' */

			appl_args->cli_file = (char *)malloc(len);
			if (appl_args->cli_file == NULL) {
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}

			strcpy(appl_args->cli_file, optarg);
			break;

		default:
			break;
		}
	}

	if (appl_args->if_count == 0) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	optind = 1;		/* reset 'extern optind' from the getopt lib */
}

/**
 * Print system and application info
 */
static void print_info(char *progname, appl_args_t *appl_args)
{
	int i;

	printf("\n"
		   "ODP system info\n"
		   "---------------\n"
		   "ODP API version: %s\n"
		   "CPU model:       %s\n"
		   "CPU freq (hz):   %lu\n"
		   "Cache line size: %i\n"
		   "Core count:      %i\n"
		   "\n",
		   odp_version_api_str(), odp_cpu_model_str(),
		   odp_cpu_hz(), odp_sys_cache_line_size(),
		   odp_cpu_count());

	printf("Running ODP appl: \"%s\"\n"
		   "-----------------\n"
		   "IF-count:        %i\n"
		   "Using IFs:      ",
		   progname, appl_args->if_count);
	for (i = 0; i < appl_args->if_count; ++i)
		printf(" %s", appl_args->if_names[i]);
	printf("\n\n");
	fflush(NULL);
}

/**
 * Prinf usage information
 */
static void usage(char *progname)
{
	printf("\n"
		   "Usage: %s OPTIONS\n"
		   "  E.g. %s -i eth1,eth2,eth3\n"
		   "\n"
		   "ODPFastpath application.\n"
		   "\n"
		   "Mandatory OPTIONS:\n"
		   "  -i, --interface Eth interfaces (comma-separated, no spaces)\n"
		   "\n"
		   "Optional OPTIONS\n"
		   "  -c, --count <number> Core count.\n"
		   "  -h, --help           Display help and exit.\n"
		   "\n", NO_PATH(progname), NO_PATH(progname)
		);
}

int run_dpdk_thread(int argc, char *argv[], int port,
    void (*recv)(const struct ofp_sockaddr_in &, void *, int))
{
    odph_odpthread_t thread_tbl[MAX_WORKERS];
	appl_args_t params;
	int core_count, num_workers;
	odp_cpumask_t cpumask;
	char cpumaskstr[64];
	odph_odpthread_params_t thr_params;
	odp_instance_t instance;

	struct rlimit rlp;
	getrlimit(RLIMIT_CORE, &rlp);
	printf("RLIMIT_CORE: %ld/%ld\n", rlp.rlim_cur, rlp.rlim_max);
	rlp.rlim_cur = 200000000;
	printf("Setting to max: %d\n", setrlimit(RLIMIT_CORE, &rlp));

	/* Parse and store the application arguments */
	parse_args(argc, argv, &params);

	if (odp_init_global(&instance, NULL, NULL)) {
		OFP_ERR("Error: ODP global init failed.\n");
		exit(EXIT_FAILURE);
	}
	if (odp_init_local(instance, ODP_THREAD_CONTROL)) {
		OFP_ERR("Error: ODP local init failed.\n");
		exit(EXIT_FAILURE);
	}

	/* Print both system and application information */
	print_info(NO_PATH(argv[0]), &params);

	core_count = odp_cpu_count();
	num_workers = core_count;

	if (params.core_count)
		num_workers = params.core_count;
	if (num_workers > MAX_WORKERS)
		num_workers = MAX_WORKERS;

	/*
	 * By default core #0 runs Linux kernel background tasks.
	 * Start mapping thread from core #1
	 */
	ofp_init_global_param(&app_init_params);

	if (core_count > 1)
		num_workers--;

	num_workers = odp_cpumask_default_worker(&cpumask, num_workers);
	odp_cpumask_to_str(&cpumask, cpumaskstr, sizeof(cpumaskstr));

	printf("Num worker threads: %i\n", num_workers);
	printf("first CPU:          %i\n", odp_cpumask_first(&cpumask));
	printf("cpu mask:           %s\n", cpumaskstr);

	app_init_params.if_count = params.if_count;
	app_init_params.if_names = params.if_names;
	app_init_params.pkt_hook[OFP_HOOK_LOCAL] = fastpath_local_hook;
	if (ofp_init_global(instance, &app_init_params)) {
		OFP_ERR("Error: OFP global init failed.\n");
		exit(EXIT_FAILURE);
	}

	memset(thread_tbl, 0, sizeof(thread_tbl));
	/* Start dataplane dispatcher worker threads */

	thr_params.start = default_event_dispatcher;
	thr_params.arg = (void *)ofp_eth_vlan_processing;
	thr_params.thr_type = ODP_THREAD_WORKER;
	thr_params.instance = instance;
	odph_odpthreads_create(thread_tbl,
			       &cpumask,
			       &thr_params);

	/* other app code here.*/
	/* Start CLI */
	ofp_start_cli_thread(instance, app_init_params.linux_core_id, params.cli_file);

    listen_port = port;
    recv_callback = recv;
	msg_queue.clear();

	/* udp server */
	ofp_start_udpserver_thread(instance, app_init_params.linux_core_id);

	odph_odpthreads_join(thread_tbl);
	printf("End Main()\n");

	return 0;
}

static void ofp_start_udpserver_thread(odp_instance_t instance, int core_id)
{
    static odph_odpthread_t udpserver_poll_pthread;
    static odph_odpthread_t udpserver_send_pthread;
	odp_cpumask_t cpumask;
	odph_odpthread_params_t poll_params;
	odph_odpthread_params_t send_params;

	odp_cpumask_zero(&cpumask);
	odp_cpumask_set(&cpumask, core_id);

	poll_params.start = poll_thread;
	poll_params.arg = NULL;
	poll_params.thr_type = ODP_THREAD_CONTROL;
	poll_params.instance = instance;
	odph_odpthreads_create(&udpserver_poll_pthread, &cpumask, &poll_params);

    send_params.start = send_thread;
	send_params.arg = NULL;
	send_params.thr_type = ODP_THREAD_CONTROL;
	send_params.instance = instance;
    odph_odpthreads_create(&udpserver_send_pthread, &cpumask, &send_params);
}

static int poll_thread(void *arg)
{
    int serv_fd;
	struct ofp_sockaddr_in my_addr;
	uint32_t my_ip_addr;
	ofp_fd_set read_fd;

	(void)arg;

	OFP_INFO("UDP server thread started");

	if (ofp_init_local()) {
		OFP_ERR("Error: OFP local init failed.\n");
		return -1;
	}
	sleep(1);

	my_ip_addr = ofp_port_get_ipv4_addr(0, 0, OFP_PORTCONF_IP_TYPE_IP_ADDR);

	if ((serv_fd = ofp_socket(OFP_AF_INET, OFP_SOCK_DGRAM, OFP_IPPROTO_UDP)) < 0) {
		OFP_ERR("ofp_socket failed, err='%s'",
			 ofp_strerror(ofp_errno));
		return -1;
	}

	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = OFP_AF_INET;
	my_addr.sin_port = odp_cpu_to_be_16(listen_port);
	my_addr.sin_addr.s_addr = my_ip_addr;
	my_addr.sin_len = sizeof(my_addr);

	if (ofp_bind(serv_fd, (struct ofp_sockaddr *)&my_addr,
		       sizeof(struct ofp_sockaddr)) < 0) {
		OFP_ERR("ofp_bind failed, err='%s'",
			 ofp_strerror(ofp_errno));
		return -1;
	}

    struct ofp_sigevent ev;
	struct ofp_sock_sigval ss;
	ss.sockfd = serv_fd;
	ss.event = 0;
	ss.pkt = ODP_PACKET_INVALID;
	ev.ofp_sigev_notify = 1;
	ev.ofp_sigev_notify_function = notify;
	ev.ofp_sigev_value.sival_ptr = &ss;
	ofp_socket_sigevent(&ev);

	OFP_FD_ZERO(&read_fd);
	OFP_FD_SET(serv_fd, &read_fd);

	while (1) {
		sleep(1);
	}

	OFP_INFO("UDP server exiting");
	return 0;
}

static int send_thread(void *)
{
	int send_fd;

	if (ofp_init_local()) {
		OFP_ERR("Error: OFP local init failed.\n");
		return -1;
	}

	if ((send_fd = ofp_socket(OFP_AF_INET, OFP_SOCK_DGRAM, OFP_IPPROTO_UDP)) < 0) {
		OFP_ERR("ofp_socket failed, err='%s'",
			 ofp_strerror(ofp_errno));
		return -1;
	}

	while (1) {
		std::unique_lock<std::mutex> lock(msg_queue_mtx);
		while (msg_queue.empty()) {
			msg_queue_cv.wait(lock);
		}
		message_t msg = msg_queue.front();
		msg_queue.pop_front();
		lock.unlock();

		ofp_socklen_t addr_len = sizeof(msg.addr);
		int nbytes = ofp_sendto(send_fd, msg.str.data(), msg.str.length(),
			0, (ofp_sockaddr *)&msg.addr, addr_len);
		
		if (nbytes < 0) {
			OFP_ERR("ofp_socket send failed, err='%s'",
				ofp_strerror(ofp_errno));
		}
	}
	
}

void send_msg(const struct ofp_sockaddr_in &addr, const std::string &str)
{
	message_t msg {
		.addr = addr,
		.str = str
	};

	std::unique_lock<std::mutex> lock(msg_queue_mtx);
	msg_queue.push_back(msg);
	lock.unlock();
	msg_queue_cv.notify_one();
}

static void notify(union ofp_sigval sv)
{
    struct ofp_sock_sigval *ss = (ofp_sock_sigval *)sv.sival_ptr;
	int s = ss->sockfd;
	int event = ss->event;
	odp_packet_t pkt = ss->pkt;
	int num_bytes;
	struct ofp_sockaddr_in addr;
	ofp_socklen_t addr_len = sizeof(addr);

	/*
	 * Only receive events are accepted.
	 */
	if (event != OFP_EVENT_RECV)
		return;
    
	uint8_t *p = (uint8_t *)ofp_udp_packet_parse(pkt, &num_bytes,
					    (struct ofp_sockaddr *)&addr,
					    &addr_len);
    
	recv_callback(addr, odp_packet_data(pkt), num_bytes);

	odp_packet_free(pkt);
	ss->pkt = ODP_PACKET_INVALID;
}