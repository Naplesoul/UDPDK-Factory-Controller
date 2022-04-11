'''
Description: 
Autor: Le Chen
Date: 2022-04-11 20:47:27
LastEditors: Weihang Shen
LastEditTime: 2022-04-11 23:58:01
'''
from http import server
import socket
import argparse
import json
import time
import threading

import random

random.seed(time.time())

location_x = 100
location_y = 100
radius = 300
block_list = []
blocks_caught = []

def send_heartbeat():
    while 1:
        str_send = json.dumps({"source": "arm", "message_type": "heartbeat", "x": location_x, "y": location_y, "r": radius})
        client_socket.sendto(str_send.encode(), (server_ip, port))
        time.sleep(0.5)

def recv_message():
    global block_list
    global blocks_caught

    while 1:
        data = client_socket.recvfrom(10240)[0]
        try:
            str_recv = json.loads(data.decode())
        except TypeError:
            print("Received Message With Wrong Format!!!")
            continue

        blocks = str_recv["blocks"]
        
        succ_vec = []
        fail_vec = []
        for block in blocks:
            # the block has already been caught before
            if block_list.__contains__(block):
                blocks_caught.append(block)
                succ_vec.append(block["id"])
                print("Block {} already caught".format(block["id"]))
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
    if random.random() > 0.5:
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
    print("Server IP: {}, port: {}".format(server_ip, port))

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    address = ("127.0.0.1", 50001)
    client_socket.bind(address)

    try:
        t1 = threading.Thread(target=send_heartbeat)
        t2 = threading.Thread(target=recv_message)
        t1.start()
        t2.start()
    except KeyboardInterrupt:
        t1.join()
        t2.join()