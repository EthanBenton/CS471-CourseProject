# Problem 2: Virtual Memory management problem

## Question

```text
--------------- VMEMMAN (subdir 2)
-------------- README file with directions (make sure to include actual commands that can be copied and pasted to run)
-------------- SOURCE code (well documented)
-------------- Executable code
-------------- Sample input data files
-------------- Sample output file(s)
----------------A report that summarizes your findings from the experiments
Compare the performance of the following page replacement algorithms: FIFO, LRU
(Least recently used), MRU (most recently used), and optimal. You are provided with a
file containing the virtual addresses that are being referenced by a single program (a
process). Run your program with the following parameters:
Page size: 512, 1024, 2048 (words)
Number of frames allocated to the process: 4, 8, 12
(So you will have 9 runs, with each page size and number of frames combination. Each
run contains statistics for each of the four page replacement algorithms.
You must collect and print the following statistics
Page Size #of pages Page replacement ALG Page fault percentage
Your report must show a summary of runs and your conclusions.
```

## Answer

### Source

[Source](/Q2VMEMMAN/src)

### Input

[Source](/Q2VMEMMAN/input/input.txt)

### Output

[Source](/Q2VMEMMAN/output)
