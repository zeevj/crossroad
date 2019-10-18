import pygame
import yaml
from time import sleep
pygame.init()
pygame.mixer.init(48000, -16, 1, 1024)


def parse(key, data):
    print("--->>>",data)
    if ".wav" in data or ".mp3" in data:
        # pygame.mixer.music.set_endevent("SONG_END")
        file = "./song/"+data
        print("opening ", file)
        pygame.mixer.music.load(file)
        # crash_sound = pygame.mixer.Sound(file)
        # pygame.mixer.Sound.play(crash_sound)
    if "play" in data:
        print("found play")
        pygame.mixer.music.play()
    if "stop" in data:
        print("found stop")
        pygame.mixer.music.stop()
    if key is not None and "step" in key:
        if "ef" in data:
            print("step",key[4:],"will trigger effect", data[2:], "when pressed")

queue = {}
timeCounterMs = 0
tickMs = 100

with open(loc + "/song/grav1.yaml", 'r') as stream:
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

while timeCounterMs < 10 * 1000:
    
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
    


