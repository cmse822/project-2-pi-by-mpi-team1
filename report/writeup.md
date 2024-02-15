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

*Output:*  
<blockquote>
  Before the 'init': Hello, World! <br>
  Before the 'init': Hello, World! <br>
  Before the 'init': Hello, World! <br>
  Before the 'init': Hello, World!
</blockquote>

*Explanation:*   
  Before executing `MPI_Init`, the four processors are running independently. Therefore, the print code  (`cout`) is executed independently by each processor once.

<u>**Between 'MPI_Init()' and 'MPI_Finalize()'**</u>

*Output:*

<blockquote>
  Between the 'init' and 'finalize': Hello, World! <br>
  Rank: 2. Size: 4 <br>
  Between the 'init' and 'finalize': Hello, World! <br>
  Rank: 0. Size: 4 <br>
  Between the 'init' and 'finalize': Hello, World! <br>
  Rank: 1. Size: 4 <br>
  Between the 'init' and 'finalize': Hello, World! <br>
  Rank: 3. Size: 4
</blockquote>

*Explanation:*  
  Once we initialize MPI, each processor has its own rank for identification(i.e. task ID). Although each processor executes the print code once again, their operations are no longer independent of each other, instead, they can communicate with each other and cooperate in parallel computation.

<u>**After 'MPI_Finalize()'**</u>  

*Output:*

<blockquote>
  After the 'finalize': Hello, World!  <br>
  After the 'finalize': Hello, World!  <br>
  After the 'finalize': Hello, World!  <br>
  After the 'finalize': Hello, World!
</blockquote>

*Explanation:*  
  Once we terminate the MPI execution environment using `MPI_Finalize()`, the four processors return to an independent state. Therefore, the print code is executed once again by each processor independently.  
<br>
  
### Q5  
Excercise 2.3, 2.4, 2.5 from Parallel Programming Book. 

#### Exercise 2.3 Use the command `MPI_Get_processor_name`. Confirm that you are able to run a program that uses two different nodes.

Rewrite `hello.cpp` as `hello_for_E23.cpp` (already uploaded) and run it on amd20 using 2 nodes, the output results are as follows. We can use `MPI_Get_processor_name` to see that the program ran on two nodes, `lac-215` and `lac-216`, respectively.

<blockquote>
This job is running on lac-215 on Tue Feb  6 22:37:16 EST 2024  <br>
Processor name: lac-215  <br>
Processor name: lac-216  <br>
Hello, World!  <br>
Rank: 0. Size: 2  <br>
Hello, World!  <br>
Rank: 1. Size: 2
</blockquote>

#### Exercise 2.4 Write a program where each process prints out a message reporting its number, and how many processes there are. Write a second version of this program, where each process opens a unique file and writes to it. On some clusters this may not be advisable if you have large numbers of processors, since it can overload the file system.

The `hello_mpi.cpp` program includes the functionality to output the rank and size. I used 4 processors, and part of the output results are as follows. We can see that the 4 processors independently output their respective results and their corresponding ranks in sequence.

<blockquote>
  Between the 'init' and 'finalize': Hello, World! <br>
  Rank: 2. Size: 4 <br>
  Between the 'init' and 'finalize': Hello, World! <br>
  Rank: 0. Size: 4 <br>
  Between the 'init' and 'finalize': Hello, World! <br>
  Rank: 1. Size: 4 <br>
  Between the 'init' and 'finalize': Hello, World! <br>
  Rank: 3. Size: 4
</blockquote>

The second version of the code is named `hello_for_E24.cpp`, and I ran the program using 2 processors. Eventually, 2 files were created with the following contents:

- rank_0.txt
<blockquote>
Hello, World. Rank: 0. Size: 2
</blockquote>

- rank_1.txt
<blockquote>
Hello, World. Rank: 1. Size: 2
</blockquote>

<br>
  
#### Exercise 2.5 Write a program where only the process with number zero reports on how many processes there are in total.

Based on the previous code, I added the conditional statement `if (rank == 0)` to complete the new code `hello_for_E25.cpp`, which meets the requirements. I used 4 processors, and the output results are as follows:

<blockquote>
Rank: 0  <br>
Total number of processes: 4  <br>
</blockquote>  


<br>
  
## Part 4: Eat Some Pi
