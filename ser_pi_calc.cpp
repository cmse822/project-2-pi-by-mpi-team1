/***************************************************************************
 * FILE: ser_pi_calc.c
 * DESCRIPTION:  
 *   Serial pi Calculation - C Version
 *   This program calculates pi using a "dartboard" algorithm.  See
 *   Fox et al.(1988) Solving Problems on Concurrent Processors, vol.1
 *   page 207.  
 * AUTHOR: unknown
 * REVISED: 02/23/12 Blaise Barney
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void srandom (unsigned seed);  
double dboard (int darts);

//#define DARTS 10000   /* number of throws at dartboard. This variable is not used in the whole program, can be ignored. */
#define ROUNDS 100    	/* number of times "darts" is iterated */

int main(int argc, char *argv[])
{
	double pi;          	/* average of pi after "darts" is thrown */
	double avepi;       	/* average pi value for all iterations */
	double receive_pi;
	int i, n;
	int tasks, rank;
	int seed;
	double darts;
	int darts_per_rank, darts_per_round;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &tasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	if (argc < 3) {
		if (rank == 0) perror("Arguments: pi seed num_samples\n");
		return -1;
	}

	seed = atoi(argv[1]);
	darts = atof(argv[2]);
	darts_per_rank = (int)(darts / (float)tasks);
	darts_per_round = darts_per_rank / ROUNDS;

	srandom (seed + rank);            /* seed the random number generator */
	avepi = 0;
	for (i = 0; i < ROUNDS; i++) {
	   /* Perform pi calculation on serial processor */
	   pi = dboard(darts_per_round);
	   avepi = ((avepi * i) + pi)/(i + 1); 
	}    
	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Reduce(&avepi, &receive_pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	receive_pi = receive_pi / (double)tasks;

	if (rank == 0) {
		printf("%f\n", receive_pi);	
	}

	MPI_Finalize();
}


/*****************************************************************************
 * dboard
 *****************************************************************************/
#define sqr(x)	((x)*(x))
long random(void);

double dboard(int darts)
{
   double x_coord,       /* x coordinate, between -1 and 1  */
          y_coord,       /* y coordinate, between -1 and 1  */
          pi,            /* pi  */
          r;             /* random number scaled between 0 and 1  */
   int score,            /* number of darts that hit circle */
       n;
   unsigned int cconst;  /* must be 4-bytes in size */
/*************************************************************************
 * The cconst variable must be 4 bytes. We check this and bail if it is
 * not the right size
 ************************************************************************/
   if (sizeof(cconst) != 4) {
      printf("Wrong data size for cconst variable in dboard routine!\n");
      printf("See comments in source file. Quitting.\n");
      exit(1);
      }
   /* 2 bit shifted to MAX_RAND later used to scale random number between 0 and 1 */
   cconst = 2 << (31 - 1); //By shifting 2 to the left by 30 positions, you effectively multiply 2 by 2^30, which equals 2^31.
   score = 0;

/***********************************************************************
 * Throw darts at board.  Done by generating random numbers
 * between 0 and 1 and converting them to values for x and y
 * coordinates and then testing to see if they "land" in
 * the circle."  If so, score is incremented.  After throwing the
 * specified number of darts, pi is calculated.  The computed value
 * of pi is returned as the value of this function, dboard.
 ************************************************************************/

   for (n = 1; n <= darts; n++) {
      /* generate random numbers for x and y coordinates */
	  /* The random() function returns a pseudo-random integer. 
   		By dividing this integer by cconst, scale it down to a float between 0 and 1 because cconst is a large integer, 
   		specifically 2^31, which is the maximum positive value for a 32-bit signed integer.*/
      r = (double)random()/cconst;
      x_coord = (2.0 * r) - 1.0;
      r = (double)random()/cconst;
      y_coord = (2.0 * r) - 1.0;

      /* if dart lands in circle, increment score */
      if ((sqr(x_coord) + sqr(y_coord)) <= 1.0)
         score++;
      }

   /* calculate pi */
   pi = 4.0 * (double)score/(double)darts;
   return(pi);
} 
