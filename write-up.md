# <p style="text-align: center;"> CMSE 822 &nbsp;&nbsp;&nbsp; SS24
## <p style="text-align: center;">  Project 2 Getting Started with MPI
<p style="text-align: right; font-size: 18px;">  &#9786; Team 1  </p>
<p style="text-align: right; font-size: 18px;">  Benjamin DalFavero, Reza Khan Mohammadi, Tairan Song, Xiaotian Hua, Ziyu Cheng</p>  



## Part 1 :  Warm-up

 


## Part 3: MPI Basics

The `hello.cpp` program will print "Hello, World!" once. The modified output result is as follows, with the specific code stored in `hello_mpi.cpp`.

### Before the 'init'
- **Output:**

<blockquote>
  Before the 'init': Hello, World! <br>
  Before the 'init': Hello, World! <br>
  Before the 'init': Hello, World! <br>
  Before the 'init': Hello, World!
</blockquote>

- **Explain:** \
  Before executing `MPI_Init`, the four processes are running independently. Therefore, the print instruction is executed once by each process independently.

### Between the 'init' and 'finalize'
- **Output:**

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

- **Explain:** \
  After completing MPI initialization, each process has its own rank for identification. Although each process runs the print instruction once again, their operations are no longer independent of each other; they can cooperate in parallel computation.

### After the 'finalize'
- **Output:**

<blockquote>
  After the 'finalize': Hello, World!  <br>
  After the 'finalize': Hello, World!  <br>
  After the 'finalize': Hello, World!  <br>
  After the 'finalize': Hello, World!
</blockquote>

- **Explain:** \
  After executing `MPI_Finalize`, the MPI task is complete, and the four processes return to an independent state. Therefore, the print instruction is executed once again by each process independently.



## Part 4: Eat Some Pi