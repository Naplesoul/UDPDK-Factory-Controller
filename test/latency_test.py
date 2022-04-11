from http import server
import socket
import argparse
import json
import time
import threading

import random

random.seed(time.time())

location_x = 500
location_y = 200

cam_x = 100
cam_y = 100

radius = 300
block_list = []
blocks_caught = []

next_id = 0

avg_latency = 0
data_cnt = 0

id_map = {}


def send_heartbeat():
    while 1:
        str_send = json.dumps(
            {"source": "arm", "message_type": "heartbeat", "x": location_x, "y": location_y, "r": radius})
        client_socket.sendto(str_send.encode(), (server_ip, port))
        time.sleep(0.5)


def send_cam_heartbeat():
    while 1:
        str_send = json.dumps(
            {"source": "camera", "message_type": "heartbeat", "x": cam_x, "y": cam_y, "w": 200, "h": 100})
        cam_socket.sendto(str_send.encode(), (server_ip, port))
        time.sleep(0.5)


def create_obj():
    while 1:
        global next_id
        str_send = json.dumps(
            {"source": "camera", "message_type": "normal", "speed": 0.01, "timestamp": str(time.time_ns()),
             "blocks": [{"id": next_id, "x": 100, "y": 100, "angle": 0}]})
        id_map[next_id] = time.time_ns()
        cam_socket.sendto(str_send.encode(), (server_ip, port))
        next_id += 1
        time.sleep(0.1)


def recv_message():
    global block_list
    global blocks_caught

    while 1:
        data = client_socket.recvfrom(65536)[0]
        cur_time = time.time_ns()
        try:
            str_recv = json.loads(data.decode())
        except TypeError:
            # print("Received Message With Wrong Format!!!")
            continue

        blocks = str_recv["blocks"]

        succ_vec = []
        fail_vec = []
        for block in blocks:
            if id_map.__contains__(block["id"]):
                global data_cnt
                global avg_latency
                send_time = id_map[block["id"]]
                latency = (cur_time - send_time) / 1000000
                avg_latency = (avg_latency * data_cnt + latency) / (data_cnt + 1)
                csv_file.write(str(latency) + "\n")
                # print("block", block["id"], "latency:", latency, "ms")
                # print("avg", avg_latency, "ms")
                print(latency)
                del id_map[block["id"]]
                data_cnt += 1
            # the block has already been caught before
            if block_list.__contains__(block):
                blocks_caught.append(block)
                succ_vec.append(block["id"])
                # print("Block {} already caught".format(block["id"]))
            else:
                if catch(block):
                    succ_vec.append(block["id"])
                    blocks_caught.append(block)
                else:
                    fail_vec.append(block["id"])

        succ_response = json.dumps({"source": "arm", "message_type": "catch_success", "block_ids": succ_vec})
        fail_response = json.dumps({"source": "arm", "message_type": "catch_fail", "block_ids": fail_vec})

        client_socket.sendto(succ_response.encode(), (server_ip, port))
        client_socket.sendto(fail_response.encode(), (server_ip, port))

        block_list.extend(blocks_caught)
        blocks_caught.clear()


def catch(block):
    return True
    if random.random() > 0.99:
        return True
    else:
        return False


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--server', type=str, default='192.168.1.101', help='server url')
    parser.add_argument('-p', '--port', type=int, default=8080)

    args = parser.parse_args()
    server_ip = args.server
    port = args.port
    # print("Server IP: {}, port: {}".format(server_ip, port))

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    cam_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # address = ("127.0.0.1", 50001)
    # client_socket.bind(address)
    csv_file = open("./data.csv", "w+")

    try:
        t1 = threading.Thread(target=send_heartbeat)
        t2 = threading.Thread(target=recv_message)
        t3 = threading.Thread(target=send_cam_heartbeat)
        t4 = threading.Thread(target=create_obj)
        t1.start()
        t2.start()
        t3.start()
        t4.start()
    except KeyboardInterrupt:
        t1.join()
        t2.join()
        t3.join()
        t4.join()
