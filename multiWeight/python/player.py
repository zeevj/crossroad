import pygame
import yaml
from time import sleep
import glob
import threading

import test_serial as myserial

sounds = [pygame.mixer.Sound] * 8

thread_read_queue = None
thread_read_from_port = None

def parse(key, data):
    print("--->>>",data)
    
    if "play" in data:
        print("found play")
        pygame.mixer.music.play()
        # pygame.mixer.Sound.play(sounds[0])
    elif "stop" in data:
        print("found stop")
        pygame.mixer.music.stop()
        # pygame.mixer.Sound.stop(sounds[0])
    elif key is not None and "step" in key:
        if "ef" in data:
            print("step",key[4:],"will trigger effect", data[2:], "when pressed")
        if ".wav" in data or ".mp3" in data:
            file = "./songs/"+data
            sounds[int(key[4:])] = pygame.mixer.Sound(file)
        if "nosound" in data:
            pygame.mixer.Sound.stop(sounds[int(key[4:])])
            sounds[int(key[4:])] = pygame.mixer.Sound
    elif "ef" in data:
        effect_number = data[2:3]
        print("run effect", effect_number)
        tokens = data.split(",")
        tokenslen = len(tokens)
        if tokenslen > 1:
            time = tokens[2]
            red = tokens[3]
            green = tokens[4]
            blue = tokens[5]  
        else:
            time = 100
            red = 255
            green = 0
            blue = 0                
        turn_on = 1
        #ef,effect_number,turn_on,time,reg,green,blue
        #ef,5,1,100,255,0,0
        cmd = "ef," + str(effect_number) + "," + str(turn_on) + "," + str(time) + "," + str(red) + "," + str(green) + "," + str(blue) + "\n"
        print("sending ", cmd)
        myserial.write(cmd.encode())
    elif ".wav" in data or ".mp3" in data:
        file = "./songs/"+data
        print("opening ", file)
        music = pygame.mixer.music.load(file)
        # sounds[0] = pygame.mixer.Sound(file)
        pygame.mixer.music.play()
        pygame.mixer.music.stop()

def read_queue(text):
    queue = {}
    timeCounterMs = 0
    tickMs = 100

    yamlList = glob.glob("./songs/*.yaml")

    with open(yamlList[0], 'r') as stream:
        try:
            queue = yaml.safe_load(stream)
            # print()

            keysToReplace = []
            for key in queue:
                if isinstance(key, (int, float, complex)):
                    keysToReplace.append(key)
            for key in keysToReplace:
                queue[int(key*1000)] = queue.pop(key)
                    # print(val)
        except yaml.YAMLError as exc:
            print(exc)

    while len(queue) > 0 :
        timeKey = queue.pop(timeCounterMs, None)
        # print("timeKey",timeCounterMs)
        if timeKey != None:
            if isinstance(timeKey,dict):
                for key in timeKey:
                    # print(timeKey.keys())
                    parse(key, timeKey[key])
            else:
                parse(None, timeKey)
        timeCounterMs = int(timeCounterMs + tickMs)
        sleep(tickMs / 1000.0) 

    myserial.stop()
    print("closing")
    
    exit(0)

def main():
    # init_serial()
    myserial.start()
    pygame.init()
    pygame.mixer.init(48000, -16, 1, 1024)
    thread_read_queue = threading.Thread(target=read_queue, daemon=True, args=("yo",))
    thread_read_queue.start()
    # while 1:
    #     print("...")
    #     sleep(1) 
    
if __name__== "__main__":
      main()