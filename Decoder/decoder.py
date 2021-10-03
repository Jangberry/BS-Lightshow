import paho.mqtt.client as mqtt
import sys
import os
import struct
import time
import creditentials
import dicts

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("/led/stream")
    client.subscribe("/led", qos=1)
    client.subscribe("/led/ping")

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload) + "\t" + str(time.time()))
    decode(msg.payload)
    

def decode(payload: bytearray):
    read = 1
    if payload[0] & 1:
        out = "BS Mode, "
        if payload[0] & 2:
            out += "inGame, "
        else:
            out += "outGame, "
        if payload[0] & 4:
            read += 1
            out += "No vanilla color change, " + "Event n°" + str(payload[1]) + " for " + dicts.zones[(payload[0] >> 3) & 0x07]
            if (payload[0] >> 6) == 1:
                read += 3
                out += ", RGB:" + colorString(payload[2], payload[3], payload[4])
            elif (payload[0] >> 6) == 2:
                read += 11
                out += ", Gradient:" + colorString(payload[7], payload[8], payload[9]) + dicts.easings[payload[2]] + colorString(payload[10], payload[11], payload[12])
        else:
            read += 3
            out += "Color change for color " + str(payload[0] >> 6) + " " + colorString(payload[1], payload[2], payload[3])
    else:
        out = "Normal Mode, "
        read += 3
        if payload[0] & 2:
            out += "zone " + dicts.zones[(payload[0] >> 3) & 0x07] + " " + colorString(payload[1], payload[2], payload[3])
        else:
            out += "uniform " + colorString(payload[1], payload[2], payload[3])
    
    print(out)
    if read < len(payload):
        decode(payload[read:])

def colorString(r, g, b):
    return f"\033[{48};2;{r};{g};{b}m   \033[0m"

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.tls_set()
client.username_pw_set(creditentials.username, password=creditentials.password)

client.connect(creditentials.host, 8883, 60)

try:
    while 1:
        client.loop()
except KeyboardInterrupt:
    pass

client.disconnect()