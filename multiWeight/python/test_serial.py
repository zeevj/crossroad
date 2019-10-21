import threading

from time import sleep
import serial
import serial.tools.list_ports
from sys import platform
import atexit

need_to_stop_tread = False
class Led:
    r = 127
    g = 127
    b = 127

    def Led(self):
        pass

    def getColorArray(self):
        return [self.r,self.g,self.b]

NUM_OF_LEDS = 10
leds = [Led] * NUM_OF_LEDS
needToWrite = True
global_serial = None

for index, led in enumerate(leds):
    leds[index] = Led()

def exit_handler():
    global global_serial
    print('######\nMy application is ending!, closing serial connection\n######' )
    if global_serial != None:
        global_serial.close()

def handle_data(data):
    if (len(data) > 0):
        data_tokens = [x.strip() for x in data.split(',')]
        if len(data_tokens) > 1:
            if "step" in data_tokens[0]:
                print("st",data_tokens[1],"is",data_tokens[2])
                print("TODO: implement logics")
            if "button" in data_tokens[0]:
                print("bt",data_tokens[1],"is",data_tokens[2])
                print("TODO: implement logics")
               
        print("data",data)
        #needToWrite = True
        
def write_data(write_data):
    # needToWrite = False
    print("write")
    for index, led in enumerate(leds):
        isCounter = index == counter
        color = 127 if isCounter else 0
        leds[index].r = color
        leds[index].g = color
        leds[index].b = color

    arr = []
    for led in leds:
        for color in led.getColorArray():
            arr.append(color)
    values = bytearray(arr)

    ser.write(START_LEDS)
    ser.write(values)
    ser.write(END_LEDS)
    # ser.write(str(counter).encode())

    counter = (counter + 1) % NUM_OF_LEDS
    
def read_from_port(txt):
    global global_serial
    global need_to_stop_tread
    
    connected = False

    while not connected:
        #serin = ser.read()
        connected = True
        START_LEDS = bytearray([0xFD,0xFE])
        END_LEDS = bytearray([0xFD,0xFF])

        counter = 0 
        cc = 0
        line = []
        # while global_serial.inWaiting():
        # print(global_serial)
        # cntr = 0
        while 1:
            if need_to_stop_tread:
                exit(0)
            while global_serial.inWaiting() > 1:
                reading = global_serial.readline().decode('utf8','ignore')
                print("read -> ",reading)
                # handle_data(reading)
                # if needToWrite:
                #     write_data(reading)

                # sleep(.1 / 10.0)  # Delay for one tenth of a second

def waitForSerial(name):
    ser = None
    n = 0
    while n < 500:
        sleep(0.1)
        print("***")
        n += 1
        comlist = serial.tools.list_ports.comports() 
        for element in comlist:
            # print(element)
            if name in element.device:
                print(element)
                ser = serial.Serial(element.device, timeout=None)
                ser.baudrate = 230400
                # ser.baudrate = 9600
                n = 600
                print("found device\n")
                return ser
def wait_for_port(port = None):
    port = ""
    if platform == "linux" or platform == "linux2":
        # linux
        port = "/dev/ttyACM"
    elif platform == "darwin":
        # OS X
        port = "USBtoUART"
    elif platform == "win32":
        # Windows...
        pass

    ser = waitForSerial(port)
    return ser
    # thread = threading.Thread(target=read_from_port, args=(ser,))
    # thread.start()

def write(txt):
    if global_serial == None:
        print("no derial connection")
        return
    global_serial.write(txt)
    
def start():
    global global_serial
    global_serial = wait_for_port()
    atexit.register(exit_handler)
    thread_read_from_port = threading.Thread(target=read_from_port, args=("bbla",))
    thread_read_from_port.start()

def stop():
    global need_to_stop_tread
    need_to_stop_tread = True

def main():
    global global_serial
    global_serial = wait_for_port()
    atexit.register(exit_handler)
    read_from_port(global_serial)
if __name__== "__main__":
      main()
