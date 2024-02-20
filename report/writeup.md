# <p style="text-align: center;"> CMSE 822 &nbsp;&nbsp;&nbsp; SS24
## <p style="text-align: center;">  Project 2 Getting Started with MPI
<p style="text-align: right; font-size: 18px;">  &#9786; Team 1  </p>
<p style="text-align: right; font-size: 18px;">  Benjamin DalFavero, Reza Khan Mohammadi, Tairan Song, Xiaotian Hua, Ziyu Cheng</p>  


## Part 1 :  Warm-up  
Exercise 2.18, 2.19, 2.21, 2.22, 2.23, 2.27 from HPSC textbook.  

### Exercise 2.18

Explain the problem with the following code:
```c
// serial initialization
for (i=0; i<N; i++)
a[i] = 0.;

#pragma omp parallel for
for (i=0; i<N; i++)
a[i] = b[i] + c[i];
```

*Explanation:*  
  The first loop is serial and it initializes the array a to 0. This means that the initialization of the entire array a is done by a single thread. And the second loop is parallel, different threads will calculate different parts of the array a.

  According to the first-touch strategy, the actual allocation of array a in memory is done when it is accessed for the first time, when other threads in the parallel loop access a, they might face higher latency if they are on different cores, leading to reduced parallel efficiency.


 Improved code:
```c
#pragma omp parallel for
for (i = 0; i < N; i++)
    a[i] = 0.;

#pragma omp parallel for
for (i = 0; i < N; i++)
    a[i] = b[i] + c[i];
```


### Exercise 2.19 
Using a chunksize of 1 leads to poor performance primarily due to false sharing. This occurs because threads frequently update adjacent memory locations (elements in the array `a[i]`), which are likely to be on the same cache line (typically 64 bytes). When multiple threads update different variables on the same cache line, it causes the cache line to be invalidated and transferred among the cores, leading to significant performance penalties. A better chunksize would be larger, potentially aligned with the size of a cache line or sufficiently large to minimize the overhead of thread management and reduce the risk of false sharing. Not specifying a chunksize and letting OpenMP manage the distribution can also be effective, as OpenMP might balance the workload across threads more efficiently.

### Exercise 2.21
```
int myTaskID, nTasks;
MPI_Comm_rank(MPI_COMM_WORLD, &myTaskID);
MPI_Comm_size(MPI_COMM_WORLD, &nTasks);

double bfromleft, bfromright; 
int leftproc = myTaskID - 1;
int rightproc = myTaskID + 1;

// Boundary condition handling
if (myTaskID == 0) leftproc = MPI_PROC_NULL;
if (myTaskID == nTasks - 1) rightproc = MPI_PROC_NULL;

MPI_Request sendreqs[2], recvreqs[2];
// Send to left and receive from right
if (leftproc != MPI_PROC_NULL) MPI_Irecv(&bfromleft, 1, MPI_DOUBLE, leftproc, 0, MPI_COMM_WORLD, &recvreqs[0]);
if (rightproc != MPI_PROC_NULL) MPI_Isend(&b[LocalProblemSize-1], 1, MPI_DOUBLE, rightproc, 0, MPI_COMM_WORLD, &sendreqs[0]);
// Send to right and receive from left
if (rightproc != MPI_PROC_NULL) MPI_Irecv(&bfromright, 1, MPI_DOUBLE, rightproc, 0, MPI_COMM_WORLD, &recvreqs[1]);
if (leftproc != MPI_PROC_NULL) MPI_Isend(&b[0], 1, MPI_DOUBLE, leftproc, 0, MPI_COMM_WORLD, &sendreqs[1]);

// Wait for all non-blocking operations to complete before proceeding with computation
if (leftproc != MPI_PROC_NULL) MPI_Wait(&recvreqs[0], MPI_STATUS_IGNORE);
if (rightproc != MPI_PROC_NULL) MPI_Wait(&sendreqs[0], MPI_STATUS_IGNORE);
if (rightproc != MPI_PROC_NULL) MPI_Wait(&recvreqs[1], MPI_STATUS_IGNORE);
if (leftproc != MPI_PROC_NULL) MPI_Wait(&sendreqs[1], MPI_STATUS_IGNORE);

// Computation part using bfromleft and bfromright

MPI_Finalize();
```


### Exercise 2.22
```
int rank, size;
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);

double send_buf = x[rank]; 
double recv_buf;

int prev_rank = (rank == 0) ? size - 1 : rank - 1;
int next_rank = (rank + 1) % size;

MPI_Request send_request, recv_request; 
MPI_Irecv(&recv_buf, 1, MPI_DOUBLE, prev_rank, 0, MPI_COMM_WORLD, &recv_request);
MPI_Isend(&send_buf, 1, MPI_DOUBLE, next_rank, 0, MPI_COMM_WORLD, &send_request);

MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
y[rank] += recv_buf; 

MPI_Wait(&send_request, MPI_STATUS_IGNORE);

MPI_Finalize();
```
#### Disadvantage of Non-blocking over Blocking 
The main disadvantage in this context is the increased complexity of the code. With non-blocking communication, we must explicitly manage the timing of communications and computations to ensure that data dependencies are respected without leading to deadlocks or race conditions. This complexity includes managing communication handles and ensuring that MPI_Wait (or other synchronization operations) is called at the correct times to guarantee that data has been properly sent and received before it is used. Additionally, non-blocking communication requires careful consideration of the workload and communication patterns to achieve the overlapping communication overlap effectively. If not properly managed the program could spend time waiting for communications to complete.

### Exercise 2.23
#### Purely Distributed Model
In a purely distributed model with MPI processes on each core, communication between nodes is handled through explicit message passing. If two MPI processes on one node need to send messages to two processes on another node, there would be four individual messages. 
Let's assume...
- Latency per message is $L$,
- Bandwidth is $B$,
- And the size of each message is $S$.

Then the time to send a message would be $$T_{message} = L + \frac{S}{B}$$, which for four messages, the total cost would be $$T_{total} = 4 \times (L + \frac{S}{B})$$

#### Hybrid Model
In the hybrid model, where OpenMP is used for intra-node communication and MPI for inter-node, messages that are sent from one node to processes on another node can be bundled together, which can reduce the number of messages to one bundled message instead of four, assuming an optimal scenario where all data can be bundled into a single message of size $4S$. In this case, the time to send this bundled message would be $$T_{total, hybrid} = L + \frac{4S}{B}$$

#### Cost Savings Analysis
The difference in total time between the purely distributed and hybrid models is:
$$T_{savings} = T_{total, distributed} - T_{total, hybrid}$$
$$T_{savings} = 4L + \frac{4S}{B} - (L + \frac{4S}{B})$$
$$T_{savings} = 3L$$

### Exercise 2.27
#### Extreme Cases
1. **Computation Time is Zero**: In this case, the entire runtime is dominated by communication. Overlapping cannot provide any benefit because there's no computation to overlap with communication. The potential gain from overlapping in this scenario is zero.

2. **Communication Time is Zero**: Here, the runtime is solely determined by computation. Since communication doesn't contribute to the runtime, overlapping is irrelevant. The gain from overlapping is also zero because there's no communication time to hide.

#### General Case
The potential gain from overlapping depends on the relative amounts of time spent on communication and computation. Now, given...
- $T_{comp}$ as the time taken for computation,
- $T_{comm}$ as the time taken for communication.

#### Without Overlapping
The total time $T_{total}$ will be $$T_{total} = T_{comp} + T_{comm}$$

#### With Overlapping
In this case, the total time $T'_{total}$ is the maximum of the two times $$T'_{total} = \max(T_{comp}, T_{comm})$$

The potential gain from overlapping in this case is $$\text{Gain} = T_{total} - T'_{total}$$

This gain is maximized when computation and communication times are similar, allowing for parts of communication to be hidden behind computation. In contrast, the gain diminishes as the imbalance between $T_{comp}$ and $T_{comm}$ increases, especially when one significantly outweighs the other.

## Part 3: MPI Basics

### Q3
Running the command `mpiexec -n 4 ./a.out` after compiling the given `hello.cpp` source file, we got "Hello, World!" printed for four times.  Because we specified numoftasks=4, on four processors, each of them executed the file `a.out` once, as a displaying result, we got "Hello, World!" printed for four times.  

### Q4  
We added the commands `MPI_Init` and `MPI_Finalize`, and put three different statements in the code, the new source file is named as `hello_mpi.cpp`. We recompiled and ran command `mpiexec -n 4 ./a.out`, output results and explanations are as follows.

<u>**Before 'MPI_Init()'**</u>

- Output:
```
  Before the 'init': Hello, World!
  Before the 'init': Hello, World!
  Before the 'init': Hello, World!
  Before the 'init': Hello, World!
```

- Explanation: 
  Before executing `MPI_Init`, the four processors are running independently. Therefore, the print code  (`cout`) is executed independently by each processor once.

<u>**Between 'MPI_Init()' and 'MPI_Finalize()'**</u>

- Output:

```
  Between the 'init' and 'finalize': Hello, World!
  Rank: 2. Size: 4
  Between the 'init' and 'finalize': Hello, World!
  Rank: 0. Size: 4
  Between the 'init' and 'finalize': Hello, World!
  Rank: 1. Size: 4
  Between the 'init' and 'finalize': Hello, World!
  Rank: 3. Size: 4
```

- Explanation:
  Once we initialize MPI, each processor has its own rank for identification(i.e. task ID). Although each processor executes the print code once again, their operations are no longer independent of each other, instead, they can communicate with each other and cooperate in parallel computation.

<u>**After 'MPI_Finalize()'**</u>  

- Output:

```
  After the 'finalize': Hello, World!
  After the 'finalize': Hello, World!
  After the 'finalize': Hello, World!
  After the 'finalize': Hello, World!
```

- Explanation:
  Once we terminate the MPI execution environment using `MPI_Finalize()`, the four processors return to an independent state. Therefore, the print code is executed once again by each processor independently.  
<br>
  
### Q5  
Excercise 2.3, 2.4, 2.5 from Parallel Programming Book. 

#### Exercise 2.3 
##### Use the command `MPI_Get_processor_name`. Confirm that you are able to run a program that uses two different nodes.

Rewrite `hello.cpp` as `hello_for_E23.cpp` (already uploaded) and run it on amd20 using 2 nodes, the output results are as follows. We can use `MPI_Get_processor_name` to see that the program ran on two nodes, `lac-215` and `lac-216`, respectively.

```
This job is running on lac-215 on Tue Feb  6 22:37:16 EST 2024  <br>
Processor name: lac-215
Processor name: lac-216
Hello, World!
Rank: 0. Size: 2
Hello, World!
Rank: 1. Size: 2
```

#### Exercise 2.4 
##### Write a program where each process prints out a message reporting its number, and how many processes there are. Write a second version of this program, where each process opens a unique file and writes to it. On some clusters this may not be advisable if you have large numbers of processors, since it can overload the file system.

The `hello_mpi.cpp` program includes the functionality to output the rank and size. I used 4 processors, and part of the output results are as follows. We can see that the 4 processors independently output their respective results and their corresponding ranks in sequence.

```
  Between the 'init' and 'finalize': Hello, World!
  Rank: 2. Size: 4
  Between the 'init' and 'finalize': Hello, World!
  Rank: 0. Size: 4
  Between the 'init' and 'finalize': Hello, World!
  Rank: 1. Size: 4
  Between the 'init' and 'finalize': Hello, World!
  Rank: 3. Size: 4
```

The second version of the code is named `hello_for_E24.cpp`, and I ran the program using 2 processors. Eventually, 2 files were created with the following contents:

- rank_0.txt
```
Hello, World. Rank: 0. Size: 2
```

- rank_1.txt
```
Hello, World. Rank: 1. Size: 2
```

  
#### Exercise 2.5 
##### Write a program where only the process with number zero reports on how many processes there are in total.

Based on the previous code, I added the conditional statement `if (rank == 0)` to complete the new code `hello_for_E25.cpp`, which meets the requirements. I used 4 processors, and the output results are as follows:

```
Rank: 0
Total number of processes: 4
``` 


<br>
  
## Part 4: Eat Some Pi
#### Q1 
Extend the `ser_pi_calc` program file using collective MPI routines to compute pi in parallel using the method described above.   
-New program file (with the same name) can be found in the Github repository.
#### Q2 
For the first iteration, perform the same number of "rounds" on each MPI rank. Measure the total runtime using `MPI_WTIME()`. Vary the number of ranks used from 1 to 4. How does the total runtime change?  
-The total runtime did not change.

#### Q3 
Divide the number of "rounds" up amongst the number of ranks using the appropriate MPI routines to decide how to distribute the work. Again, run the program on 1 to 4 ranks. How does the runtime vary now?  
-The total runtime decreased.

#### Q4 
Change the number of "darts" and ranks. Use your MPI program to compute `pi` using total numbers of "darts" of 1E3, 1E6, and 1E9\. For each dart count, run your code on HPCC with processor counts of 1, 2, 4, 8, 16, 32, and 64\. Keep track of the resulting value of `pi` and the runtimes. Use non-interactive jobs and modify the `submitjob.sb` script as necessary. 

#### Q5 
For each processor count, plot the resulting errors in your computed values of `pi` compared to the true value as functions of the number of darts used to compute it. Use log-log scaling on this plot. What is the rate of convergence of your algorithm for computing `pi`? Does this value make sense? Does the error or rate of convergence to the correct answer vary with processor count? Should it?   
-See comments below.

#### Q6 
For each dart count, make a plot of runtime versus processor count. Each line represents a "strong scaling" study for your code. For each dart count, also plot the "ideal" scaling line. Calculate the parallel scaling efficiency of your code for each dart count. Does the parallel performance vary with dart count? Explain your answer.    
-See comments below.

#### Q7 
Try running your code on different node types on HPCC with varied core counts. In particular, try to ensure that your runs utilize multiple nodes so that the communication network is used. Do you see a change in the communication cost when the job is run on more than one node?  

*(The code of plotting could be found at '/results/runtime_plot.ipynb'
)*

![time_nodes](/results/time_scaling.png "time_scaling")
#### **Comments**
- We observed interesting patterns from the plot.  
  Perform the same number of 'rounds' on each rank, when the number of darts is small ($10^3, 10^6$), as the number of nodes increases, the total runtime slightly increases; when the number of darts is larger ($10^9$), as the number of nodes increases, the total runtime obviously decreases.  
     
- This observed behavior is influenced by a balance between **computational workload** and **parallel overhead among the nodes**. In the context of parallel computing, "overhead" refers to the extra work and time required to manage the parallel processes. It includes the time spent on starting up and shutting down the parallel environment, communicating between nodes, synchronizing processes, and any additional operations that are necessary only because the task is being run in parallel rather than sequentially.  

- For a small number of darts, the workload done by each node is quite small, and the overhead of initializing MPI environment, distributing workload among nodes, synchronization(mpi_barrier) and communication(mpi_reduce) can dominate the actual run time. Each node spends more time waiting for synchronization and communication than doing the actual computation. Therefore, there is less workload than overhead, there is not enough computational workload to benefit from the parallel structure, leading to the patterns we observed when num_of_nodes are 10^3, 10^6: the total run time slightly increases as number of nodes increases.  

- For a large number of darts, the workload done by each node is substantial enough so that the time for actual computation is significantly comparable to the overhead time. To be more specific, each node can perform a significant amount of actual computation before need to synchronize or communicate with other nodes. In this case, when the total workload is fixed, as the parallel resources(num_of_nodes) increases, the total run time decreases, which showcases the benefits of parallel computing.

- In summary, what we learned is that for parallel computing, there is a crossover point where the increase in parallel resources starts to show benefits(in this problem, parallel resources are the number of nodes). Below this point, overheads donimate; above this point, actual computation can efficiently utilize the parallel structure.


![error_nundarts](/results/error_scaling.png "error_scaling")

#### **Comments**
- We converted *num_of_darts* and *error* into log scale and then computed the slope of the relationship between num_of_darts and the error of estimated Pi value.  

- Slope interpretation  
1) Negativity:   
The negative slope of the line shows that as the number of darts increases, the error decreases.  
  
2) Slope value:   
In an ideal Monte Carlo simulation for estimating Pi, the error should scale with $ \frac{1}{\sqrt{n}}$ where n is num_of_darts, due to *the law of large numbers theory*. This is because the standard deviation of the sample mean in Monte Carlo simulation decreases with $\frac{1}{\sqrt{n}}$ where n is the sample size. In the log-log scale, this relationship would be expected to have a slope of $ -\frac{1}{2}$. In the plot, this ideal $ -\frac{1}{2}$ slope line is shown in pink named 'ideal'.

3) By comparing the slope of lines that drawn from our empirical results with the ideal slope of $ -\frac{1}{2}$, we can assess how well the simulation performs. The closer the empirical slopes are to $ -\frac{1}{2}$, the better the simulation is bahaving as theoretically expected. Our empirical results show that our mean_slope = -.04555 and std_slope = 0.0246, which implies that our simulation of estimating Pi behaves as it is theoretically expected :)









