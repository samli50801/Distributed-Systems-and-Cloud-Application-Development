# start-up script
import os
import time
import urllib.request
import boto3
import msgpackrpc

def new_client(ip, port):
    return msgpackrpc.Client(msgpackrpc.Address(ip, port))

#
my_node_ip = urllib.request.urlopen("http://169.254.169.254/latest/meta-data/public-ipv4").read().decode()
#my_node_ip = '127.0.0.1'
existing_node_ip = my_node_ip # by default

ec2 = boto3.resource('ec2', "us-east-1")
INSTANCE_STATE = 'running'
instances = ec2.instances.filter(
    Filters=[
        {
            'Name': 'instance-state-name',
            'Values': [
                INSTANCE_STATE
            ]
        }
    ]
)

first_iter = True
first_inst = False
for instance in instances:
    if first_iter and instance.public_ip_address == my_node_ip:
        first_inst = True
        first_iter = False
    if instance.public_ip_address != my_node_ip:
        existing_node_ip = instance.public_ip_address
        break

print("my_node_ip: ", my_node_ip)
print('existing_node_ip: ', existing_node_ip)

# Start Chord node
os.system("/root/chord-part-2/chord {} 5057 &".format(my_node_ip))
# Join existing Chord system
my_chord_client = new_client(my_node_ip, 5057)
existing_chord_client = new_client(existing_node_ip, 5057)

time.sleep(20)

if first_inst:
    my_chord_client.call("create")
else:
    my_chord_client.call("join", existing_chord_client.call("get_info"))

time.sleep(20)

