
# Process Scheduler with Memory Allocation

Consider a Computer with 1-CPU and infinite memory. It is required to make a
scheduler with its complementary components as sketched in the following diagram.

![flow diagram](https://github.com/DanielGebraiel/Process-Scheduler-with-Memory-Allocation/blob/main/screenshots/diagram.png)



## Features

-Read the input files (check the input/output section below).\
-Asks the user for the chosen scheduling algorithm and its parameters, if there
are any.

-Generate three files: (check the input/output section below)
- Scheduler.log
- Scheduler.perf 
- memory.log

-Report the following information (In the .perf file)
- CPU utilization.
- Average weighted turnaround time.
- Average waiting time.
- Standard deviation for average weighted turnaround time.

-The total memory size is 1024 bytes.\
-Each process size is less than or equal 256 bytes.



## Implemented Algorithms
- Non-preemptive Highest Priority First (HPF).
- Shortest Remaining time Next (SRTN).
- Round Robin (RR).

## Example (Round Robin with quantum=2)

-Comments start with '#' and are not ready by the program

### Sample input file

```txt
#id arrival runtime priority memSize
1   1   2   2   90
2   2   6   9   256 
3   3   4   6   1 
4   4   1   3   129 
5   5   9   1   234 

```

### scheduler.log file

```txt
#At time x process y state arr w total z remain y wait k
At time 1 process 1 started arr 1 total 2 remain 2 wait 0
At time 3 process 1 finished arr 1 total 2 remain 0 wait 0 TA 2 WTA 1.00
At time 3 process 2 started arr 2 total 6 remain 6 wait 1
At time 5 process 2 stopped arr 2 total 6 remain 4 wait 1
At time 5 process 3 started arr 3 total 4 remain 4 wait 2
At time 7 process 3 stopped arr 3 total 4 remain 2 wait 2
At time 7 process 4 started arr 4 total 1 remain 1 wait 3
At time 8 process 4 finished arr 4 total 1 remain 0 wait 3 TA 4 WTA 4.00
At time 8 process 5 started arr 5 total 9 remain 9 wait 3
At time 10 process 5 stopped arr 5 total 9 remain 7 wait 3
At time 10 process 2 resumed arr 2 total 6 remain 4 wait 6
At time 12 process 2 stopped arr 2 total 6 remain 2 wait 6
At time 12 process 3 resumed arr 3 total 4 remain 2 wait 7
At time 14 process 3 finished arr 3 total 4 remain 0 wait 7 TA 11 WTA 2.75
At time 14 process 6 started arr 6 total 7 remain 7 wait 8
At time 16 process 6 stopped arr 6 total 7 remain 5 wait 8
At time 16 process 7 started arr 7 total 2 remain 2 wait 9
At time 18 process 7 finished arr 7 total 2 remain 0 wait 9 TA 11 WTA 5.50
At time 18 process 5 resumed arr 5 total 9 remain 7 wait 11
At time 20 process 5 stopped arr 5 total 9 remain 5 wait 11
At time 20 process 2 resumed arr 2 total 6 remain 2 wait 14
At time 22 process 2 finished arr 2 total 6 remain 0 wait 14 TA 20 WTA 3.33
At time 22 process 6 resumed arr 6 total 7 remain 5 wait 14
At time 24 process 6 stopped arr 6 total 7 remain 3 wait 14
At time 24 process 5 resumed arr 5 total 9 remain 5 wait 15
At time 26 process 5 stopped arr 5 total 9 remain 3 wait 15
At time 26 process 6 resumed arr 6 total 7 remain 3 wait 16
At time 28 process 6 stopped arr 6 total 7 remain 1 wait 16
At time 28 process 5 resumed arr 5 total 9 remain 3 wait 17
At time 30 process 5 stopped arr 5 total 9 remain 1 wait 17
At time 30 process 6 resumed arr 6 total 7 remain 1 wait 18
At time 31 process 6 finished arr 6 total 7 remain 0 wait 18 TA 25 WTA 3.57
At time 31 process 5 resumed arr 5 total 9 remain 1 wait 18
At time 32 process 5 finished arr 5 total 9 remain 0 wait 18 TA 27 WTA 3.00
```

### scheduler.perf file

```txt
CPU utilization = 100.00%
Avg WTA = 3.31
Avg Waiting = 9.86
Std WTA = 1.26
```

### memory.log file

```txt
#At time x allocated y bytes for process z from i to j
At time 1 allocated 256 bytes for process 1 from 0 to 255
At time 2 allocated 240 bytes for process 2 from 256 to 511
At time 3 freed 256 bytes for process 1 from 0 to 255
At time 3 allocated 199 bytes for process 3 from 0 to 255
At time 4 allocated 129 bytes for process 4 from 512 to 767
At time 5 allocated 234 bytes for process 5 from 768 to 1023
At time 8 freed 129 bytes for process 4 from 512 to 767
At time 8 allocated 128 bytes for process 6 from 512 to 639
At time 8 allocated 19 bytes for process 7 from 640 to 671
At time 14 freed 199 bytes for process 3 from 0 to 255
At time 18 freed 19 bytes for process 7 from 640 to 671
At time 22 freed 240 bytes for process 2 from 256 to 511
At time 31 freed 128 bytes for process 6 from 512 to 639
At time 32 freed 234 bytes for process 5 from 768 to 1023

```
## Lessons Learned

- Writing a complex program such as a scheduler in C
- Using Interprocess communication methods for sending data between processes (message queues, shared memory)
- Synchronization between processes
- Using various data structures to make the program modular
- Testing and debugging in C
