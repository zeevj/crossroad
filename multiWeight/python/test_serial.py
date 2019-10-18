from time import sleep
import serial
import serial.tools.list_ports

from sys import platform

import atexit

def exit_handler():
    print('######\nMy application is ending!, closing serial connection\n######' )
    ser.close()
    
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
                ser.baudrate = 9600
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
atexit.register(exit_handler)

# '/dev/ttyACM0'
# ser = serial.Serial('/dev/ttyACM0', 9600)
counter = 32  # Below 32 everything in ASCII is gibberish

while True:
    counter += 1
    # Convert the decimal number to ASCII then send it to the Arduino
    ser.write(str(counter).encode())
    print(ser.readline())  # Read the newest output from the Arduino
    sleep(.1)  # Delay for one tenth of a second
    if counter == 255:
        counter = 32
