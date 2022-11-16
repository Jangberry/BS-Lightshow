import glob
import sys
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
        except (OSError, serial.SerialException):
            pass
    return result


class LedsZones:
    def __init__(self, leds: list) -> None:
        """Manage LEDs accordingly to how they're managed on the µC side

        Args:
            leds (list[int]): List of nb of leds by zones
        """
        self.__leds = leds


class eventsSender:
    def __init__(self, port = None) -> None:
        if port is None:
            port = list_serial_ports()[0]
        self.__conn = serial.Serial(port, 115200, write_timeout=1)

    def send(self, message):
        buf = bytearray(0)
        buf.append(int(len(message)))
        self.__conn.write(buf+message)
        print(message.hex())

    def __del__(self):
        self.__conn.close()

