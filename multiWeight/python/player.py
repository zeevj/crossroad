import pygame
import yaml
from time import sleep
import glob

sounds = [pygame.mixer.Sound] * 8

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
    elif "ef" in data:
        print("run effect", data[2:])
    elif ".wav" in data or ".mp3" in data:
        file = "./songs/"+data
        print("opening ", file)
        music = pygame.mixer.music.load(file)
        # sounds[0] = pygame.mixer.Sound(file)
        pygame.mixer.music.play()
        pygame.mixer.music.stop()

def main():
    pygame.init()
    pygame.mixer.init(48000, -16, 1, 1024)

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

    while len(queue) > 0:
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

if __name__== "__main__":
      main()


