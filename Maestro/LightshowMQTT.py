import paho.mqtt.client as mqtt
import json
import time
import simpleaudio as sa
import sys
import os
import struct
import creditentials
import easingdict

path = sys.argv[1]
print(path)

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.tls_set()
client.username_pw_set(creditentials.username, password=creditentials.password)

client.connect(creditentials.host, 8883, 60)


try:
    info = open(path + "/Info.dat")
except FileNotFoundError:
    info = open(path + "/info.dat")
infos = json.load(info)
#file = open(path + "\\" + infos["_difficultyBeatmapSets"][0]["_difficultyBeatmaps"][0]["_beatmapFilename"])
file = open(path + "/" + infos["_difficultyBeatmapSets"][0]["_difficultyBeatmaps"][0]["_beatmapFilename"])
track = json.load(file)

bpm = float(infos["_beatsPerMinute"])

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_start()

blankbuf = 7
bufs=[]
times = []

for i in range(len(track["_events"])):
    if track["_events"][i]["_type"] < 6:
        if track["_events"][i]["_time"] not in times:
            times.append(track["_events"][i]["_time"])
            bufs.append(bytearray(0))
        thisbuf = bytearray(2)
        thisbuf[0] += blankbuf + (int(track["_events"][i]["_type"]) << 3)
        thisbuf[1] += min(int(track["_events"][i]["_value"]), 255)
        if "_customData" in track["_events"][i]:
            if "_color" in track["_events"][i]["_customData"]:
                thisbuf[0] += 1 << 6
                thisbuf.append(int(min(track["_events"][i]["_customData"]["_color"][0] * 255, 255)))
                thisbuf.append(int(min(track["_events"][i]["_customData"]["_color"][1] * 255, 255)))
                thisbuf.append(int(min(track["_events"][i]["_customData"]["_color"][2] * 255, 255)))
            if "_lightGradient" in track["_events"][i]["_customData"]:
                thisbuf[0] += 2 << 6
                thisbuf.append(easingdict.easings[track["_events"][i]["_customData"]["_lightGradient"]["_easing"]])
                thisbuf += struct.pack('f', track["_events"][i]["_customData"]["_lightGradient"]["_duration"])# * 60 / bpm) This factor isn't used in the mod and it seems to be more correct...
                thisbuf.append(int(min(track["_events"][i]["_customData"]["_lightGradient"]["_startColor"][0] * 255, 255)))
                thisbuf.append(int(min(track["_events"][i]["_customData"]["_lightGradient"]["_startColor"][1] * 255, 255)))
                thisbuf.append(int(min(track["_events"][i]["_customData"]["_lightGradient"]["_startColor"][2] * 255, 255)))
                thisbuf.append(int(min(track["_events"][i]["_customData"]["_lightGradient"]["_endColor"][0] * 255, 255)))
                thisbuf.append(int(min(track["_events"][i]["_customData"]["_lightGradient"]["_endColor"][1] * 255, 255)))
                thisbuf.append(int(min(track["_events"][i]["_customData"]["_lightGradient"]["_endColor"][2] * 255, 255)))
        bufs[times.index(track["_events"][i]["_time"])] += thisbuf

buf = bytearray(0)
if "_customData" in infos["_difficultyBeatmapSets"][0]["_difficultyBeatmaps"][0]:
    customdata = infos["_difficultyBeatmapSets"][0]["_difficultyBeatmaps"][0]["_customData"]
    if "_envColorLeft" in customdata and "_envColorRight" in customdata:
        buf.append(int(0b00000001))
        buf.append(min(int(customdata["_envColorLeft"]['r'] * 255), 255))
        buf.append(min(int(customdata["_envColorLeft"]['g'] * 255), 255))
        buf.append(min(int(customdata["_envColorLeft"]['b'] * 255), 255))
        buf.append(int(0b01000001))
        buf.append(min(int(customdata["_envColorRight"]['r'] * 255),255))
        buf.append(min(int(customdata["_envColorRight"]['g'] * 255),255))
        buf.append(min(int(customdata["_envColorRight"]['b'] * 255),255))
    else:
        buf.append(int(0b00000001))
        buf.append(int(255))
        buf.append(int(0))
        buf.append(int(0))
        buf.append(int(0b01000001))
        buf.append(int(0))
        buf.append(int(0))
        buf.append(int(255))
else:     
    buf.append(int(0b00000001))
    buf.append(int(1))
    buf.append(int(1))
    buf.append(int(255))
    buf.append(int(0b01000001))
    buf.append(int(255))
    buf.append(int(1))
    buf.append(int(1))
        
if len(buf) > 0:
    print(buf)
    client.publish("/led", buf, qos=1)

try:
    time.sleep(10)
    if os.name == "nt":
        os.system("bash -c 'ffmpeg -n -i \"" + path + "/" + infos["_songFilename"] + "\" \"" + path + "/" + infos["_songFilename"].replace(".egg", ".wav").replace(".ogg", ".wav") + "\"'")
    else:
        os.system("ffmpeg -n -i \"" + path + "/" + infos["_songFilename"] + "\" \"" + path + "/" + infos["_songFilename"].replace(".egg", ".wav").replace(".ogg", ".wav") + "\"")
    client.publish("/led", b"\x07\x00", qos=1)
    print(b"\x07\x03")
    time.sleep(4)
    wave_object = sa.WaveObject.from_wave_file(path + "/" + infos["_songFilename"].replace(".egg", ".wav").replace(".ogg", ".wav"))
    play_object = wave_object.play()
    start = time.monotonic()

    for i in range(len(times)):
        while time.monotonic() < start + 60 * times[i] / bpm :
            time.sleep(0.001)
        client.publish("/led/stream", bufs[i], qos=0)
        print(bufs[i].hex())
    if len(times) > 5:  # Waits only if the song has a significant amount of lighting events
        play_object.wait_done()
    time.sleep(3)
except KeyboardInterrupt:
    print("Skipping the rest of the sound")

client.publish("/led", b"\x05\x00", qos=1)
print(b"\x05\x00")
time.sleep(3)
client.disconnect()