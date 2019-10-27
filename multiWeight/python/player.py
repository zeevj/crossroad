import pygame
import yaml
from time import sleep
import glob
import threading
from colour import Color

import test_serial as myserial

sounds = [pygame.mixer.Sound] * 8
songs_yamls = ["goodbye_yellow_brkrd","chacarron","masa","artbat","bj"]  #,"test"
myserial.songs_yamls = songs_yamls



thread_read_from_port = None
thread_read_queue = None

def play_song_num(num):
    thread_read_queue = threading.Thread(target=read_queue, daemon=True, args=(num,))
    thread_read_queue.start()

def parse(key, data):
    print("--->>>",data)

    data_tokens = [item.strip() for item in data.split(',')]

    if len(data_tokens) == 1:
        #this is maybe a spacial word like stop
        pass

    send_data = False
    effect = -1
    time = 100
    red = 255
    green = 0
    blue = 0
    turn_on = 1

    if key != "button1" and key != "action":
        for tok in data_tokens:
            # if k is not None and "step" in k:
            #     dt = [item.strip() for item in data.split(',')]
            #     if "ef" in data:
            #     print("step",k[4:],"will trigger effect", data[2:], "when pressed")
            # if ".wav" in data or ".mp3" in data:
            #     file = "./songs/"+data
            #     sounds[int(k[4:])] = pygame.mixer.Sound(file)
            # if "nosound" in data:
            #     pygame.mixer.Sound.stop(sounds[int(k[4:])])
            #     sounds[int(k[4:])] = pygame.mixer.Sound

            first_char = tok[0]
            the_rest = tok[1:]

            if "e" == first_char:
                effect = int(the_rest)
                send_data = True
            elif "t" == first_char:
                time = int(the_rest)
            elif "c" == first_char:
                #pip3 install colour
                color = Color(the_rest)
                red = int(color.red*255)
                green = int(color.green*255)
                blue = int(color.blue*255)
            elif "r" == first_char:
                red = int(the_rest)
            elif "g" == first_char:
                green = int(the_rest)
            elif "b" == first_char:
                blue = int(the_rest)
            elif "o" == first_char:
                turn_on = int(the_rest)
    if send_data:
        cmd = "ef," + str(effect) + "," + str(turn_on) + "," + str(time) + "," + str(red) + "," + str(green) + "," + str(blue) + "\n"
        print("sending ", cmd)
        myserial.write(cmd.encode())
    
    if "play" in data:
        print("found play")
        pygame.mixer.music.play()
        # pygame.mixer.Sound.play(sounds[0])
    elif "stop" in data:
        print("found stop")
        pygame.mixer.music.stop()
        # pygame.mixer.Sound.stop(sounds[0])
    # elif key is not None and "step" in key:
    #     if "ef" in data:
    #         print("step",key[4:],"will trigger effect", data[2:], "when pressed")
    #     if ".wav" in data or ".mp3" in data:
    #         file = "./songs/"+data
    #         sounds[int(key[4:])] = pygame.mixer.Sound(file)
    #     if "nosound" in data:
    #         pygame.mixer.Sound.stop(sounds[int(key[4:])])
    #         sounds[int(key[4:])] = pygame.mixer.Sound
    """
    elif "ef" in data:
        tokens = data_tokens
        tokenslen = len(tokens)
        if tokenslen > 3:
            time = tokens[2]
            if "red" in tokens[3]:
                red = 255
                green = 0
                blue = 0
            elif "blue" in tokens[3]:
                red = 0
                green = 0
                blue = 255
            elif "green" in tokens[3]:
                red = 0
                green = 255
                blue = 0
            elif "black" in tokens[3]:
                red = 0
                green = 0
                blue = 0    
            elif tokenslen > 5:
                red = tokens[3]
                green = tokens[4]
                blue = tokens[5]  
        else:
            time = 100
            red = 255
            green = 0
            blue = 0
        turn_on = 1
        effect_number = tokens[1]
        #ef,effect_number,turn_on,time,reg,green,blue
        #ef,5,1,100,255,0,0
        cmd = "ef," + str(effect_number) + "," + str(turn_on) + "," + str(time) + "," + str(red) + "," + str(green) + "," + str(blue) + "\n"
        print("sending ", cmd)
        myserial.write(cmd.encode())
    """
    # elif ".wav" in data or ".mp3" in data:
    if ".wav" in data or ".mp3" in data:
        file = "./songs/"+data
        print("opening ", file)
        music = pygame.mixer.music.load(file)
        while pygame.mixer.get_busy():
            sleep(0.01) 
        print("music",music)
        # sounds[0] = pygame.mixer.Sound(file)
        pygame.mixer.music.play()
        pygame.mixer.music.stop()

    

def read_queue(current_song):
    #while 1:
        
        queue = {}
        timeCounterMs = 0
        tickMs = 10

        offset = 0

        #yamlList = glob.glob("./songs/.yaml")
        with open("./songs/" + songs_yamls[current_song] + ".yaml", 'r') as stream:
            try:
                queue = yaml.safe_load(stream)
                # print()
                offs = queue.pop("offset", None)
                if offs != None:
                    offset = offs
                keysToReplace = []
                for key in queue:
                    if isinstance(key, (int, float, complex)):
                        keysToReplace.append(key)
                for key in keysToReplace:
                    key_time = key
                    if key_time > 0:
                        key_time += offset
                    key_time = int((key_time)*100) * 10
                    print(key_time)
                    queue[key_time] = queue.pop(key)
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

        pygame.mixer.music.stop()
        myserial.currently_playing = False
        return
    #myserial.stop()
    #print("closing")
    #exit(0)

def main():
    myserial.start()
    pygame.init()
    pygame.mixer.init(48000, -16, 1, 1024)
    
    # while 1:
    #     print("...")
    #     sleep(1) 
    
if __name__== "__main__":
      main()