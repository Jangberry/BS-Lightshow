import json
import time
import sys
import os
import struct
import glob
from asyncio import sleep, run
import simpleaudio as sa
import easingdict
import serial


def list_serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException): pass
    return result


def connect(port):
    if port is None:
        port = list_serial_ports()[0]
    return serial.Serial(port, 115200, write_timeout=1)


def send(message, ser):
    buf = bytearray(0)
    buf.append(int(len(message)))
    ser.write(buf+message)
    #print(message.hex())


def parseFile(track):
    blankbuf = 7
    bufs = []
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
                    thisbuf.append(
                        int(min(track["_events"][i]["_customData"]["_color"][0] * 255, 255)))
                    thisbuf.append(
                        int(min(track["_events"][i]["_customData"]["_color"][1] * 255, 255)))
                    thisbuf.append(
                        int(min(track["_events"][i]["_customData"]["_color"][2] * 255, 255)))
                elif "_lightGradient" in track["_events"][i]["_customData"]:
                    thisbuf[0] += 2 << 6
                    thisbuf.append(
                        easingdict.easings[track["_events"][i]["_customData"]["_lightGradient"]["_easing"]])
                    thisbuf += struct.pack('f', track["_events"][i]["_customData"]["_lightGradient"]["_duration"])
                    thisbuf.append(int(min(
                        track["_events"][i]["_customData"]["_lightGradient"]["_startColor"][0] * 255, 255)))
                    thisbuf.append(int(min(
                        track["_events"][i]["_customData"]["_lightGradient"]["_startColor"][1] * 255, 255)))
                    thisbuf.append(int(min(
                        track["_events"][i]["_customData"]["_lightGradient"]["_startColor"][2] * 255, 255)))
                    thisbuf.append(int(min(
                        track["_events"][i]["_customData"]["_lightGradient"]["_endColor"][0] * 255, 255)))
                    thisbuf.append(int(min(
                        track["_events"][i]["_customData"]["_lightGradient"]["_endColor"][1] * 255, 255)))
                    thisbuf.append(int(min(
                        track["_events"][i]["_customData"]["_lightGradient"]["_endColor"][2] * 255, 255)))
                
                if "_lightID" in track["_events"][i]["_customData"] and int(track["_events"][i]["_type"]) != 5:
                    try:
                        thisbuf.append(len(track["_events"][i]["_customData"]["_lightID"]))
                    except TypeError:
                        thisbuf.append(1)
                        thisbuf.append(int(track["_events"][i]["_customData"]["_lightID"]))
                    else:
                        if thisbuf[-1] == 1:
                            thisbuf.append(int(track["_events"][i]["_customData"]["_lightID"][0]))
                        else:
                            for j in range(thisbuf[-1]):
                                thisbuf.append(int(track["_events"][i]["_customData"]["_lightID"][j]))
                elif "_propID" in track["_events"][i]["_customData"] and int(track["_events"][i]["_type"]) != 5:
                    try:
                        thisbuf.append(len(track["_events"][i]["_customData"]["_propID"]))
                    except TypeError:
                        thisbuf.append(1)
                        thisbuf.append(int(track["_events"][i]["_customData"]["_propID"]))
                    else:
                        if thisbuf[-1] == 1:
                            thisbuf.append(int(track["_events"][i]["_customData"]["_propID"][0]))
                        else:
                            for j in range(thisbuf[-1]):
                                thisbuf.append(int(track["_events"][i]["_customData"]["_propID"][j]))
                elif int(track["_events"][i]["_type"]) != 5:
                    thisbuf.append(0)
            elif int(track["_events"][i]["_type"]) != 5:
                thisbuf.append(0)
                
            index = times.index(track["_events"][i]["_time"])
            
            if len(bufs[index]) + len(thisbuf) > 250:
                otherIndex = times[::-1].index(track["_events"][i]["_time"])
                if len(bufs[otherIndex]) + len(thisbuf) > 250:
                    times.append(track["_events"][i]["_time"])
                    bufs.append(bytearray(0))
                    otherIndex = -1
                index = otherIndex

            bufs[index] += thisbuf

    return times, bufs


def getColorBuf(infoJson, inGame):
    buf = bytearray(0)
    if "_customData" in infoJson["_difficultyBeatmapSets"][0]["_difficultyBeatmaps"][0]:
        customdata = infoJson["_difficultyBeatmapSets"][0]["_difficultyBeatmaps"][0]["_customData"]
        if "_envColorLeft" in customdata and "_envColorRight" in customdata:
            buf.append(int(0b00000001 + (2*inGame)))
            buf.append(min(int(customdata["_envColorLeft"]['r'] * 255), 255))
            buf.append(min(int(customdata["_envColorLeft"]['g'] * 255), 255))
            buf.append(min(int(customdata["_envColorLeft"]['b'] * 255), 255))
            buf.append(int(0b01000001 + (2*inGame)))
            buf.append(min(int(customdata["_envColorRight"]['r'] * 255), 255))
            buf.append(min(int(customdata["_envColorRight"]['g'] * 255), 255))
            buf.append(min(int(customdata["_envColorRight"]['b'] * 255), 255))

    if len(buf) == 0:
        buf.append(int(0b00000001 + (2*inGame)))
        buf.append(int(0))
        buf.append(int(0))
        buf.append(int(255))
        buf.append(int(0b01000001 + (2*inGame)))
        buf.append(int(255))
        buf.append(int(0))
        buf.append(int(0))
        
    f = open("LightIDTables/"+infoJson["_environmentName"]+".json")
    lights = json.load(f)
    f.close()
    for i in range(len(lights)):
        if i > 4: break
        buf.append(197+2*inGame+(i<<3))
        buf.append(len(lights[str(i)]))

    return buf


class SongStopped(Exception):
    pass


async def playLightshow(times, bufs, bpm, conn, play_object):
    start = time.monotonic()
    try:
        n = len(times)
        for i in range(n):
            while time.monotonic() < start + times[i] * 60 / bpm:
                if not play_object.is_playing():
                    raise SongStopped
                await sleep(min(0.5, start + times[i] * 60 / bpm - time.monotonic()))
            send(bufs[i], conn)
            await sleep(0.01 * min((len(bufs[i]) + len(bufs[min(i+1, n-1)]))/400, 1.1))
    except SongStopped:
        pass


def loadInfos(path, inGame=False):
    try:
        info = open(path + "/Info.dat", encoding="utf8")
    except FileNotFoundError:
        info = open(path + "/info.dat", encoding="utf8")
    infos = json.load(info)
    beatmap = open(path + "/" + infos["_difficultyBeatmapSets"]
                   [0]["_difficultyBeatmaps"][0]["_beatmapFilename"], encoding="utf8")
    times, bufs = parseFile(json.load(beatmap))
    info.close()
    beatmap.close()
    return times, bufs, float(infos["_beatsPerMinute"]), infos["_songFilename"], getColorBuf(infos, inGame)


def prepare_Wave_File(path, songFilename):
    command = "ffmpeg -n -i \"" + path + "/" + songFilename + "\" \"" + path + \
        "/" + songFilename.replace(".egg",
                                   ".wav").replace(".ogg", ".wav") + "\""
    if os.name == "nt":
        command = "bash -c \""+command.replace("\"", "\\\"")+"\""
    os.system(command.replace("&", "^&"))
    return sa.WaveObject.from_wave_file(
        path + "/" + songFilename.replace(".egg", ".wav").replace(".ogg", ".wav"))


def play(path, conn):
    times, bufs, bpm, songFilename, colorBuf = loadInfos(path)

    send(colorBuf, conn)

    wave = prepare_Wave_File(path, songFilename)

    send(b"\x07\x00\x00", conn)
        
    try:
        time.sleep(1)

        run(playLightshow(times, bufs, bpm, conn, wave.play()))

        time.sleep(1)
    except KeyboardInterrupt:
        print("Skipping the rest of the sound")

    time.sleep(1)
    send(b"\x05\x00\x00", conn)


if __name__ == '__main__':
    try:
        file = sys.argv[1].replace('\\', '/')
    except IndexError:
        print("Veuillez indiquer le chemin vers un dossier")
        file = "Music/Beat Saber"
    try:
        serport = sys.argv[2]
    except IndexError:
        try:
            serport = list_serial_ports()[0]
        except IndexError:
            serport=""
            print("Pas de péripherique série reconnu, on va tenter tout de même mais il y aura surement une erreur...")
    serconn = connect(serport)
    play(file, serconn)
    serconn.close()
