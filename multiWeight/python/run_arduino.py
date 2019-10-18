from time import sleep
import serial
import signal
import subprocess
import os


class Ramzor:
    kill_now = False

    def __init__(self, song_pid=0):
        self._song_pid = song_pid
        signal.signal(signal.SIGTERM, self.exit_gracefully)

    def exit_gracefully(self, signum, frame):
        os.killpg(self._song_pid, signal.SIGTERM)
        self.kill_now = True

    def set_song_pid(self, song_pid):
        self._song_pid = song_pid


if __name__ == '__main__':

    ser = serial.Serial('/dev/ttyUSB0', 115200)
    ser.close()
    ser.open()
    ramzor = Ramzor()

    print ''
    print 'Serial open: '
    print ser.is_open

    send_serial = True
    song_num = "1"

    while send_serial:
        ser.write(song_num)
        read = str(ser.readline())
        print 'Arduino sent: ' + read
        if read[:1] == song_num:
            send_serial = False

    print 'Play song 1'
    song_1 = "/home/yquinn/Music/1.mp3"
    message = subprocess.Popen("mpg123 -q " + song_1,
                               preexec_fn=os.setsid,
                               shell=True,
                               close_fds=True)

    ramzor.set_song_pid(message.pid)

    while not ramzor.kill_now:
        pass

    ser.close()
    print 'Serial open: '
    print ser.is_open
