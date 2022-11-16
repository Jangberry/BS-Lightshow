from Tools import *
import glob
import sys
import serial
import easingdict
from asyncio import sleep, run

"""
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
                
            
"""

if __name__ == "__main__":
    eventsSender(None, LedsZones([50, 50, 100, 100, 1]))