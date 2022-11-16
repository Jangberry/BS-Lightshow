import asyncio
import os
import threading
import sys
import csv
import serial
import LightshowUSB
import glob


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

class SongDescriptor:
    def __init__(self, path: str) -> None:
        self.path = path
        self.info = None
        self.song = None
        self.playObject = None

    def load(self, inGame: bool = True) -> None:
        """Load the infos about the song

        :param bool inGame: Decide wether the color change should be sent as ingame event or not
        """
        self.info = LightshowUSB.loadInfos(self.path, inGame)

    def prepare(self):
        """Prepare the wave file (convert from ogg to wav and load)"""
        if self.info is None:
            print("unloaded")
            self.load()
        self.song = LightshowUSB.prepare_Wave_File(self.path, self.info[3])

    def play(self, conn: serial.Serial):
        """Play the sound along with the lightshow

        :param Serial conn: serial connection to send infos on
        """
        if self.song is None:
            print("unprepared")
            self.prepare()
        LightshowUSB.send(self.info[4], conn)
        self.playObject = self.song.play()
        asyncio.run(LightshowUSB.playLightshow(
            self.info[0], self.info[1], self.info[2], conn, self.playObject))

# Is a playlist, stores a name and a list of songs


class Playlist:
    def __init__(self):
        self._songList = []

    # Checks if the playlist contains a specific song instance
    def containsSong(self, song: SongDescriptor) -> bool:
        for i in self._songList:
            if i.path == song.path:
                return True
        return False

    # Adds a song instance to the playlist
    def addAudio(self, song: SongDescriptor, next_song=False):
        if next_song:
            self._songList.insert(1, song)
        else:
            self._songList.append(song)

    # Removes a song instance from the playlist
    def removeAudio(self, path:str):
        for i in self._songList:
            if i.path == path:
                self._songList.remove(i)
                
    def removeAudios(self, nb:int):
        for i in range(nb):
            self._songList.remove(1)

    @property
    def songList(self):
        return self._songList

    @songList.setter
    def songList(self, songs: list):
        self._songList = songs


def main(playlist: Playlist, ser: serial.Serial):
    while (i := len(playlist.songList)) > 0:
        threads = []
        threads.append(threading.Thread(
            target=playlist.songList[0].play, args=(ser,), daemon=True))
        if i > 1:
            threads.append(threading.Thread(
                target=playlist.songList[1].prepare))
            if i > 2:
                threads.append(threading.Thread(
                    target=playlist.songList[2].load))
        for thread in threads:
            if not thread.is_alive():
                thread.start()
        while 1:
            try:
                while playlist.songList[0].playObject is None:
                    pass
                playlist.songList[0].playObject.wait_done()
            except KeyboardInterrupt:
                try:
                    print("Next music", playlist.songList[1].path)
                except ValueError:
                    print("This is the last song")
                print("Current music", playlist.songList[0].path)
                will = input(
                    "Wanna play an other song ? Hit enter to skip this one, type a number of music to skip or type a path to play it next: ")
                if will == "":
                    playlist.songList[0].playObject.stop()
                elif will.isnumeric():
                    playlist.songList[0].playObject.stop()
                    try:
                        playlist.removeAudios(int(will)-1)
                    except ValueError:
                        print("All the playlist have been skipped")
                elif os.path.exists(will):
                    playlist.addAudio(SongDescriptor(will), True)
                    threads.append(threading.Thread(
                        target=playlist.songList[1].prepare))
                    threads[-1].start()
                else:
                    print("This path doesn't exists, doing nothing...")
                continue
            break
        for thread in threads:
            thread.join()
        playlist.songList.pop(0)


if __name__ == '__main__':
    liste = Playlist()
    try:
        file = sys.argv[1].replace('\\', '/')
    except IndexError:
        file = "playlist.csv"
    try:
        with open(file, encoding="utf-8-sig") as csvfile:
            obj = csv.reader(csvfile)
            for row in obj:
                for i in row:
                    liste.addAudio(SongDescriptor(i))
    except FileNotFoundError:
        print("Veuillez indiquer un fichier CSV vers une playlist")
        exit()
    try:
        serport = sys.argv[2]
    except IndexError:
        try:
            serport = list_serial_ports()[0]
        except IndexError:
            serport=""
            print("Pas de péripherique série reconnu, on va tenter tout de même mais il y aura surement une erreur...")
    conn = LightshowUSB.connect(serport)
    main(liste, conn)
    conn.close()
