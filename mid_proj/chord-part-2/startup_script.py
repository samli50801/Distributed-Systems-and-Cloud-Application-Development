# start-up script
import os
import time
import urllib.request
import boto3
import msgpackrpc

def new_client(ip, port):
	return msgpackrpc.Client(msgpackrpc.Address(ip, port))

my_node_ip = urllib.request.urlopen("http://169.254.169.254/latest/meta-data/public-ipv4").read().decode()
existing_node_ip = ''

time.sleep(10)

ec2 = boto3.resource('ec2', "us-east-1")
instances = ec2.instances.all()
for instance in instances: 
    if instance.public_ip_address is None:
        continue
    if instance.public_ip_address != my_node_ip:
        existing_node_ip = instance.public_ip_address
        break
	
print("my_node_ip: ", my_node_ip)
print('existing_node_ip: ', existing_node_ip)
# Start Chord node
os.system("/root/chord-part-2/chord {} 5057 &".format(my_node_ip))
# Join existing Chord system
#my_chord_client = new_client(my_node_ip, 5057)
time.sleep(5)
#existing_chord_client = new_client(existing_node_ip, 5057)
#my_chord_client.call("join", existing_chord_client.call("get_info"))
