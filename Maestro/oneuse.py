import struct
import serial
import time
import math
import Realtime.Tools as tools

es = tools.eventsSender()
i = 0
blankbuf = 7

while 1:
    es.send(bytearray([blankbuf+((i % 5) << 3)+(2 << 6), 
                       4,
                       3]) + 
            struct.pack('f', 5.0) + 
            bytearray([int(max(min(255, 127 + 127 * math.sin(2*math.pi*(i/1000 + (i % 5)/10 + 0/3)), 255), 0)),         int(max(min(255, 127 + 127 * math.sin(2*math.pi*(i/1000 + (i % 5)/10 + 1/3)), 255), 0)),        int(max(min(255, 127 + 127 * math.sin(2*math.pi*(i/1000 + (i % 5)/10 + 2/3)), 255), 0))]) + 
            bytearray([int(max(min(255, 127 + 127 * math.sin(2*math.pi*(i/1000 + ((i+1) % 5)/10 + 0/3)), 255), 0)),     int(max(min(255, 127 + 127 * math.sin(2*math.pi*(i/1000 + ((i+1) % 5)/10 + 1/3)), 255), 0)),    int(max(min(255, 127 + 127 * math.sin(2*math.pi*(i/1000 + ((i+1) % 5)/10 + 2/3)), 255), 0)), 
                       0]))
    es.send(bytearray([blankbuf+(5 << 3)+(1 << 6), 4, 255, 255, 255, 0]))
    time.sleep(5)
