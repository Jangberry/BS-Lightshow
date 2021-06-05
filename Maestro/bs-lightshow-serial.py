import serial
import json
import time
import simpleaudio as sa
import sys
import os

path = sys.argv[1]
print(path)

ser = serial.Serial('/dev/ttyUSB0', 500000)

#info = open(path + "\\Info.dat")
try:
    info = open(path + "/Info.dat")
except FileNotFoundError:
    info = open(path + "/info.dat")
infos = json.load(info)
#file = open(path + "\\" + infos["_difficultyBeatmapSets"][0]["_difficultyBeatmaps"][0]["_beatmapFilename"])
file = open(path + "/" + infos["_difficultyBeatmapSets"][0]["_difficultyBeatmaps"][0]["_beatmapFilename"])
track = json.load(file)

bpm = float(infos["_beatsPerMinute"])

blankbuf = 7
bufs=[]
times = []

for i in range(len(track["_events"])):
    if track["_events"][i]["_type"] < 6:
        if track["_events"][i]["_time"] not in times:
            times.append(track["_events"][i]["_time"])
            bufs.append(bytearray(0))
        thisbuf = bytearray(2)
        bufs[times.index(track["_events"][i]["_time"])].append(blankbuf + (int(track["_events"][i]["_type"]) << 3))
        bufs[times.index(track["_events"][i]["_time"])].append(int(track["_events"][i]["_value"]))

#buf = b"\x01\xFF\x00\x00\x01\x00\x00\xFF"   # Defaults colors This doesn't work
buf = bytearray(0)
buf.append(int(0b00000001))
buf.append(int(255))
buf.append(int(0))
buf.append(int(0))
buf.append(int(0b01000001))
buf.append(int(0))
buf.append(int(0))
buf.append(int(255))
if "_customData" in infos["_difficultyBeatmapSets"][0]["_difficultyBeatmaps"][0]:
    customdata = infos["_difficultyBeatmapSets"][0]["_difficultyBeatmaps"][0]["_customData"]
    
    if "_colorLeft" in customdata:
        buf.append(int(0b00000001))
        buf.append(min(int(customdata["_colorLeft"]['r'] * 255),    255))
        buf.append(min(int(customdata["_colorLeft"]['g'] * 255),    255))
        buf.append(min(int(customdata["_colorLeft"]['b'] * 255),    255))
    if "_envColorLeft" in customdata:
        buf.append(int(0b00000001))
        buf.append(min(int(customdata["_envColorLeft"]['r'] * 255), 255))
        buf.append(min(int(customdata["_envColorLeft"]['g'] * 255), 255))
        buf.append(min(int(customdata["_envColorLeft"]['b'] * 255), 255))
    if "_colorRight" in customdata:
        buf.append(int(0b01000001))
        buf.append(min(int(customdata["_colorRight"]['r'] * 255),   255))
        buf.append(min(int(customdata["_colorRight"]['g'] * 255),   255))
        buf.append(min(int(customdata["_colorRight"]['b'] * 255),   255))
    if "_envColorRight" in customdata:
        buf.append(int(0b01000001))
        buf.append(min(int(customdata["_envColorRight"]['r'] * 255),255))
        buf.append(min(int(customdata["_envColorRight"]['g'] * 255),255))
        buf.append(min(int(customdata["_envColorRight"]['b'] * 255),255))
        
if len(buf) > 0:
    ser.write(buf)
print(buf)

try:
    time.sleep(5)
    #os.system("bash -c 'ffmpeg -n -i \"" + path.replace("\\", "/") + "/" + infos["_songFilename"] + "\" \"" + path.replace("\\", "/") + "/" + infos["_songFilename"].replace(".egg", ".wav").replace(".ogg", ".wav") + "\"'")
    os.system("ffmpeg -n -i \"" + path + "/" + infos["_songFilename"] + "\" \"" + path + "/" + infos["_songFilename"].replace(".egg", ".wav").replace(".ogg", ".wav") + "\"")
    ser.write(b"\x07\x03")
    time.sleep(3)
    os.chdir(path)
    wave_object = sa.WaveObject.from_wave_file(infos["_songFilename"].replace(".egg", ".wav").replace(".ogg", ".wav"))
    play_object = wave_object.play()
    start = time.monotonic()

    for i in range(len(times)):
        while time.monotonic() < start + 60 * times[i] / bpm :
            time.sleep(0.001)
        ser.write(bufs[i])
        print(bufs[i].hex())
    if len(times) > 5:  # Waits only if the song has a significant amount of lighting events
        play_object.wait_done()
    time.sleep(3)
except KeyboardInterrupt:
    print("Skipping the rest of the sound")

ser.write(b"\x05\x00")
time.sleep(3)
ser.close()