import os
from subprocess import Popen, PIPE, TimeoutExpired
import time
import pandas as pd

DIRECTORY = os.path.realpath(os.path.dirname(__file__))
EXEC_NAME = "outer.exe"

# Compile program
os.chdir(DIRECTORY)
if os.path.exists(EXEC_NAME):
    os.remove(EXEC_NAME)
os.system(f"g++ bruteforce_no_relable.cpp -o {EXEC_NAME}")

number_of_features = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
depths = [1, 2]
number_of_instances = [100, 200, 400, 600, 800, 1000]
adversary_attack_power = [0.005, 0.01, 0.02, 0.03, 0.04, 0.05]
data = []
timeouts = []
def run_with_input(filename):
    
    # timeouts[filename] = []

    f = open(f"{DIRECTORY}/datasets/{filename}.in")
    extraLines = f.read()
    f.close()
    for featureAmount in number_of_features:
        print(f"\033[2mRunning test {filename}: featureAmount: {featureAmount}\033[0m")
        for depth in depths:
            print(f"\033[2mRunning test {filename}: featureAmount: {featureAmount}, depth: {depth}\033[0m")
            for instanceAmount in number_of_instances:
                for attackPower in adversary_attack_power:
                    found = False
                    for t in timeouts:
                        if t[0] <= featureAmount and t[1] <= depth and t[2] <= instanceAmount and t[2]*t[3] <= attackPower * instanceAmount:
                            #write something in the file idk like add something to the dataframe
                            # print("found timeout")
                            data.append([filename, featureAmount, depth, instanceAmount, attackPower, -1, -1])
                            found = True
                            break
                    if found:
                        continue
                    lines = f"{featureAmount} {depth} {instanceAmount} {attackPower}" + extraLines
                    # print(lines)
                    proc = Popen([f"./{EXEC_NAME}"], stdin=PIPE, stdout=PIPE)
                    try:
                        start_time = time.time()
                        out, _ = proc.communicate(input=lines.encode("UTF-8"), timeout = 300)
                    except TimeoutExpired:
                        proc.kill()
                        # print([filename, featureAmount, depth, instanceAmount, attackPower, -1])
                        data.append([filename, featureAmount, depth, instanceAmount, attackPower, -1, -1])
                        timeouts.append([featureAmount, depth, instanceAmount, attackPower - 0.001])
                        continue
                    wrong = -1
                    # print(out)
                    try:
                        wrong = int(out)
                        if(wrong == -5):
                            data.append([filename, featureAmount, depth, instanceAmount, attackPower, -5, -5])
                        else:
                            data.append([filename, featureAmount, depth, instanceAmount, attackPower, time.time() - start_time, wrong])
                    except:
                        data.append([filename, featureAmount, depth, instanceAmount, attackPower, -2, -2])
                        print("\033[31;1mCouldn't parse output:\033[0m", str(out))
                        print([filename, featureAmount, depth, instanceAmount, attackPower, -2])
                        # print("\033[31;1mWrong result in test {filename}: featureAmount: {featureAmount}, depth: {depth}, instanceAmount: {instanceAmount}, attackPower: {attackPower}\033[0m")
                        continue

                    # f = open(f"{DIRECTORY}/datasets/{filename}.ans")
                    # max_wrong = int(f.read())
                    # f.close()

                    # if wrong == max_wrong:
                    #     print(f"\033[32;1mCorrect result in test {filename}: needed at most {max_wrong} wrong, got {wrong} wrong\033[0m")
                    #     return True
                    # else:
                    #     print(f"\033[31;1mWrong result in test {filename}: needed at most {max_wrong} wrong, got {wrong} wrong\033[0m")
                    #     return False

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
    df = pd.DataFrame(data, columns=['name', 'featureAmount', 'depth', 'instanceAmount', 'attackPower', 'runtime', 'wrong'])
    df.to_csv("bruteforcedataWithScore.csv", index=False)
    # c+=1
    # if(c > 2):
    #     break
if wrong_results:
    print("\n\033[31;1mWrong results in the following tests:\033[0m")
    for name in wrong_results:
        print(f"\033[31;1m  - {name}\033[0m")
else:
    print("\n\033[32;1mSuccess!\033[0m")
df = pd.DataFrame(data, columns=['name', 'featureAmount', 'depth', 'instanceAmount', 'attackPower', 'runtime', 'wrong'])
df.to_csv("bruteforcedataWithScore.csv", index=False)

print(f"\033[34;1mFinished all tests in {time.time() - start_time:.3f} seconds\033[0m")