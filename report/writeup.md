# <p style="text-align: center;"> CMSE 822 &nbsp;&nbsp;&nbsp; SS24
## <p style="text-align: center;">  Project 2 Getting Started with MPI
<p style="text-align: right; font-size: 18px;">  &#9786; Team 1  </p>
<p style="text-align: right; font-size: 18px;">  Benjamin DalFavero, Reza Khan Mohammadi, Tairan Song, Xiaotian Hua, Ziyu Cheng</p>  


## Part 1 :  Warm-up  
Exercise 2.18, 2.19, 2.21, 2.22, 2.23, 2.27 from HPSC textbook.  

#### Exercise 2.18

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


#### Exercise 2.19 Let’s say there are t threads, and your code looks like
```c
for (i=0; i<N; i++) {
a[i] = // some calculation
}
```
If you specify a chunksize of 1, iterations 0, t, 2t, . . . go to the first thread, 1, 1 + t, 1 +
2t, . . . to the second, et cetera. Discuss why this is a bad strategy from a performance
point of view. Hint: look up the definition of false sharing. What would be a good
chunksize?



<br>

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
