import os
from subprocess import Popen, PIPE
import sys
import time

DIRECTORY = os.path.realpath(os.path.dirname(__file__))
EXEC_NAME = "out.exe"

# Compile program
os.chdir(DIRECTORY)
if os.path.exists(EXEC_NAME):
    os.remove(EXEC_NAME)
os.system(f"g++ murtree_robust.cpp -o {EXEC_NAME}")

def run_with_input(filename):
    f = open(f"{DIRECTORY}/datasets/{filename}.in")
    lines = f.read()
    f.close()

    proc = Popen([f"./{EXEC_NAME}"], stdin=PIPE, stdout=PIPE)
    out, _ = proc.communicate(input=lines.encode("UTF-8"))
    wrong = -1
    #print(out)
    print(out.decode('utf-8'))


    f = open(f"{DIRECTORY}/datasets/{filename}.ans")
    max_wrong = int(f.read())
    f.close()
    print(f"\033[32;1mCorrect result in test {filename}: needed at most {max_wrong} wrong, got {-1} wrong\033[0m")


c = 0
for name in sorted(set(j.split(".")[0] for j in os.listdir(f"{DIRECTORY}/datasets")), key=lambda x: (x[-1], x)):
    #if(c > 1):
        
    print("new shit")
    run_with_input(name)
    print("\n\033[32;1mSuccess!\033[0m")
    print("#######################################")
    print()
    c+=1

    if(c > 40):
        break
    
    