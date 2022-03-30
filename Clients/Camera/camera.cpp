/*
 * @Description: 
 * @Autor: Weihang Shen
 * @Date: 2022-03-18 13:32:09
 * @LastEditors: Weihang Shen
 * @LastEditTime: 2022-03-22 21:31:19
 */
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <jsoncpp/json/json.h>
#include "opencv2/opencv.hpp"

#include "BlockTracker.h"

int main(int argc,char *argv[])
{
    int config_fd = open(argv[1], O_RDONLY|O_NDELAY);
    if (config_fd < 0) {
        printf("Fail to open config json file: %s", argv[1]);
        return -1;
    }

    struct stat *config_stat = (struct stat *)malloc(sizeof(struct stat));
    stat(argv[1], config_stat);
    char *config_buf = (char *)malloc(config_stat->st_size);
    int read_count = read(config_fd, config_buf, config_stat->st_size);
    if (read_count != config_stat->st_size) {
        printf("Fail to read from config json file: %s", argv[1]);
        return -1;
    }
    
    close(config_fd);
    free(config_stat);

    int client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    Json::Reader reader;
    Json::Value config_json;

    reader.parse(config_buf, config_json, false);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr =
        inet_addr(config_json["server_ip"].asCString());
    server_addr.sin_port =
        htons(config_json["server_port"].asInt());
    socklen_t sock_len = sizeof(server_addr);

    cv::VideoCapture capture(0);
    cv::Mat frame;
    BlockTracker tracker(config_json);
    sleep(1);

    do {
        capture.read(frame);
    } while (!tracker.calibrate(frame));

    std::string heartbeat = tracker.toHeartbeatString();
    sendto(client_fd, heartbeat.data(), heartbeat.length(), 0,
           (struct sockaddr *)&server_addr, sock_len);
    std::cout << heartbeat << std::endl;

    sleep(1);

    uint32_t block_nums[3];
    uint32_t frame_num = 0;
    while (capture.read(frame)) {
        tracker.update(frame, std::chrono::system_clock::now());
        if ((block_nums[frame_num++ % 3] = tracker.blockNum()) > 0 &&
            block_nums[0] == block_nums[1] &&
            block_nums[1] == block_nums[2]) {
            
            std::string json_str = tracker.toString();
            std::cout << json_str << std::endl;
            sendto(client_fd, json_str.data(), json_str.length(), 0,
                   (struct sockaddr *)&server_addr, sock_len);
        } else {
            sendto(client_fd, heartbeat.data(), heartbeat.length(), 0,
                   (struct sockaddr *)&server_addr, sock_len);
        }
        usleep(50000);
    }
}