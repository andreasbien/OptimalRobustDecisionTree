import os
from subprocess import Popen, PIPE
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
    print(out)
    try:
        wrong = int(out)
    except:
        print("\033[31;1mCouldn't parse output:\033[0m", str(out))
        exit()

    f = open(f"{DIRECTORY}/datasets/{filename}.ans")
    max_wrong = int(f.read())
    f.close()

    if wrong == max_wrong:
        print(f"\033[32;1mCorrect result in test {filename}: needed at most {max_wrong} wrong, got {wrong} wrong\033[0m")
        return True
    else:
        print(f"\033[31;1mWrong result in test {filename}: needed at most {max_wrong} wrong, got {wrong} wrong\033[0m")
        return False

start_time = time.time()

# Run tests
wrong_results = []
c =0
for name in sorted(set(j.split(".")[0] for j in os.listdir(f"{DIRECTORY}/datasets")), key=lambda x: (x[-1], x)):
    print(f"\n\033[2mRunning test {name}\033[0m")
    curr_time = time.time()
    if not run_with_input(name):
        wrong_results.append(name)
    print(f"\033[34;1mFinished in {time.time() - curr_time:.3f} seconds\033[0m")
    c+=1
    if(c > 30):
        break
if wrong_results:
    print("\n\033[31;1mWrong results in the following tests:\033[0m")
    for name in wrong_results:
        print(f"\033[31;1m  - {name}\033[0m")
else:
    print("\n\033[32;1mSuccess!\033[0m")

print(f"\033[34;1mFinished all tests in {time.time() - start_time:.3f} seconds\033[0m")