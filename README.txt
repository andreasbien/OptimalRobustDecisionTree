To run all datasets, use "run_cpp.py" and specify the algorithm you want to use in line 13 

line 13: os.system(f"g++ murtree_robust.cpp -o {EXEC_NAME}").

You can replace  "murtree_robust.cpp" with any other algorithm.

You can decide the name of the file in line 109.

experiments.ipynb contains a couple of code blocks that are used to generate the charts.


readDatasets.cpp is a helper function that you can use if you want to run the algorithm on it's c++ file. Note, you will then need to comment out some code that reads input and replace it with some hard-coded values.

I will be updating this code. If you want to assess this work before the report was submitted, just check out the last commit on the 25th of June.
