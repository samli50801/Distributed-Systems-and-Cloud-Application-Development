#!/usr/bin/python3

import msgpackrpc
import time

def new_client(ip, port):
	return msgpackrpc.Client(msgpackrpc.Address(ip, port))

client = []
for i in range(15):
	client.append(new_client("127.0.0.1", 3000+i))

for i in range(15): 
	print(client[i].call("get_info"))

for i in range(15): 
	if i == 0: 
		client[i].call("create")
	else: 
		client[i].call("join", client[i-1].call("get_info"))
	time.sleep(2)

time.sleep(20)

print("---------------------test_script start to find---------------------")
for i in range(15):
	print(client[i].call("find_successor", 100000000*i))
	time.sleep(2)
print("----------------------test_script end to find----------------------")

#14: 3359520921

client[14].call("kill")
