import threading

from time import sleep
import serial
import serial.tools.list_ports
from sys import platform
import atexit

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

for index, led in enumerate(leds):
    leds[index] = Led()

def exit_handler():
    print('######\nMy application is ending!, closing serial connection\n######' )
    ser.close()

def handle_data(data):
    if (len(data) > 0):
        print(data)
        needToWrite = True

def read_from_port(ser):
    global needToWrite
    connected = False

    while not connected:
        #serin = ser.read()
        connected = True
        START_LEDS = bytearray([0xFD,0xFE])
        END_LEDS = bytearray([0xFD,0xFF])

        counter = 0 
        cc = 0

        while True:
            cc += 1
            print(cc)
            print("read")
            reading = ser.readline().decode('utf8','ignore')
            handle_data(reading)
            if needToWrite:
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

            sleep(.1 / 10.0)  # Delay for one tenth of a second

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
                ser = serial.Serial(element.device, timeout=1)
                ser.baudrate = 230400
                n = 600
                print("found device\n")
                return ser

port = ""
if platform == "linux" or platform == "linux2":
    # linux
    port = "/dev/ttyACM0"
elif platform == "darwin":
    # OS X
    port = "USBtoUART"
elif platform == "win32":
    # Windows...
    pass

ser = waitForSerial(port)

# thread = threading.Thread(target=read_from_port, args=(ser,))
# thread.start()
read_from_port(ser)

atexit.register(exit_handler)