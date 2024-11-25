# Problem 1: Producer-Consumer Problem

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

### VirtualMemory.cpp

#### 1. Introduction

**Objective**:  
To implement a function that will evaluate the operation of different Page Replacement algorithms. Specifically the First In First Out ('FIFO'), Least Recently Used('LRU'), the Most Recently Used('MRU'), and the Optimal ('Optimal') page replacement algorithms. These algorithms will be tested via one set of input data. The impact of adjusting the available 'Frames' and 'Page Size' (in Bytes) on the performance of the algorithms.
**Scope**:  
This report:

1. Analyzes performance data collected across 9 simulation runs.
2. Provides insights and recommendations for scalability and optimization.

---

##### 2. Simulation Results

##### Configurations Tested

- **Number of Frames (`Frames`)**: 4, 8, 12
- **Page Size (`Page size`)**: 512, 1024, 2048
- Total combinations: 9 (3 × 3)

##### Efficiency Results

| **Page Size** | **#  Pages** | **#  Frames** |**Algorithm** | **Fault %** |
|----------------------|--------------------|------|--------------|---------|
| 512                  | 196                | 4    | FIFO         | 80.37   |
| 512                  | 196                | 4    | LRU          | 80      |
| 512                  | 196                | 4    | MRU          | 93.1    |
| 512                  | 196                | 4    | Optimal      | 56.63   |
| 512                  | 196                | 8    | FIFO         | 61      |
| 512                  | 196                | 8    | LRU          | 60.1    |
| 512                  | 196                | 8    | MRU          | 91.5    |
| 512                  | 196                | 8    | Optimal      | 34.23   |
| 512                  | 196                | 12   | FIFO         | 42.97   |
| 512                  | 196                | 12   | LRU          | 42.07   |
| 512                  | 196                | 12   | MRU          | 88.97   |
| 512                  | 196                | 12   | Optimal      | 21.2    |
| 1024                 | 98                 | 4    | FIFO         | 61.4    |
| 1024                 | 98                 | 4    | LRU          | 60.47   |
| 1024                 | 98                 | 4    | MRU          | 86.03   |
| 1024                 | 98                 | 4    | Optimal      | 37.9    |
| 1024                 | 98                 | 8    | FIFO         | 23.6    | 
| 1024                 | 98                 | 8    | LRU          | 22.8    |
| 1024                 | 98                 | 8    | MRU          | 81.03   |
| 1024                 | 98                 | 8    | Optimal      | 11.27   |
| 1024                 | 98                 | 12   | FIFO         | 3.57    |
| 1024                 | 98                 | 12   | LRU          | 3.57    |
| 1024                 | 98                 | 12   | MRU          | 77.27   |
| 1024                 | 98                 | 12   | Optimal      | 3.4     |
| 2048                 | 49                 | 4    | FIFO         | 26.67   |
| 2048                 | 49                 | 4    | LRU          | 26.03   |
| 2048                 | 49                 | 4    | MRU          | 73.4    |
| 2048                 | 49                 | 4    | Optimal      | 13.97   |
| 2048                 | 49                 | 8    | FIFO         | 1.9     |
| 2048                 | 49                 | 8    | LRU          | 1.9     |
| 2048                 | 49                 | 8    | MRU          | 66.3    |
| 2048                 | 49                 | 8    | Optimal      | 1.73    |
| 2048                 | 49                 | 12   | FIFO         | 1.83    |
| 2048                 | 49                 | 12   | LRU          | 1.83    |
| 2048                 | 49                 | 12   | MRU          | 60.4    |
| 2048                 | 49                 | 12   | Optimal      | 1.63    |


##### Performance Trends

1. **Impact of Increasing Frames**:
   - Increasing Frames reduced the average page fault percentage on all algorithms.

2. **Impact of Increasing Page Size**:
   - Increasing page size reduced the average page fault percentage on all algorithms

3. **Overall trends**:
   - Page size and amount of of frames available had roughly the same impact on fault percentage. 
   - The FIFO, LRU, and Optimal page replacement algorithms trended down quickly with additional frames and larger page sizes
   -The MRU algorithm was not very efficient for this data set.

---

#### 4. Insights

##### Key Insights

1.**More is better**:
  -Without regard to impacts on systems resources, the algorithms all increase their efficiency with larger page sizes and frames available.
  
2. **Different algorithms work better than others**:
   -Not all algorithms operated with the same efficiency, even with a large page size and number of pages available.
   
4. **Limits of testing**:
   -This testing was only performed on one set of data. While MRU algorithm was least effective, only one data set was tested. To get a more accurate representation of true performance, different data sets with different randomness would be best.
   
---

#### 5. Conclusion

This project demonstrated the functionality and efficiency of the different algorithms. Overall the Optimal algorithm performed the best. Due to the optimal algorithms ability to "see the future" it can understandably perform better than the alternatives. This ability to see the future is not a very reasonable expectation for most applications however. The MRU algorithm performed notably worse. This is to be expected as that algorithm does not perform well unless a page will be repeatedly used in a row and then not used again. The data also shows that you will get diminishing returns on percentage of faults when the page size and number of frames increased. With a page size of 2048 Bytes, increasing from 8 to 12 available frames improves efficiency on average by less than .1%. Conversly, increasing from 8 to 12 frames available with a page size of 512 improves efficiency on average by about 15%.

---

### Compile, Execute, & Output

If you wish to compile question one from this directory run the command

`g++ src\PageReplacement.cpp -o executable\PageReplacement`

The executable will the be located in the [executable](/Q2VMEMMAN/executable/) directory, you can then run the executable wiith the following commands:

Linux:

`./executable/PageReplacement.exe`

Windows:

`.\executable\PageReplacement.exe`

The output will be located in the [output](/Q2VMEMMAN/output/results.txt) file and the `console`

### Executable Directory

[executable](/Q2VMEMMAN/executable)

### Source Directory

[src](/Q2VMEMMAN/src)

### Input Directory

[input](/Q2VMEMMAN/input)

### Output Directory

[output](/Q2VMEMMAN/output)
