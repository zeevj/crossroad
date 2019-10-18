from time import sleep
import serial

# Establish the connection on a specific port

ser = serial.Serial('/dev/ttyACM0', 9600)
counter = 32  # Below 32 everything in ASCII is gibberish

while True:
    counter += 1
    # Convert the decimal number to ASCII then send it to the Arduino
    ser.write(str(chr(counter)))
    print ser.readline()  # Read the newest output from the Arduino
    sleep(.1)  # Delay for one tenth of a second
    if counter == 255:
        counter = 32
