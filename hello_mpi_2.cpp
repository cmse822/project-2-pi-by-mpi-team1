#include <iostream>
// required MPI include file
#include <mpi.h>

using namespace std;

int main(int argc, char *argv[]) 
{
    int numtasks, rank, len, rc;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    cout << "Before 'MPI_Init': Hello, World!" << endl; //cout: output data to the console
                                                            //<<  : insertion operator, send data to the output stream (cout)
                                                            //endl: end current line and move to the beginning of next line

    // initialize MPI
    MPI_Init(&argc, &argv); 

    // get number of tasks
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks); 
    // get my rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 

    cout << "Between 'MPI_Init' and 'MPI_Finalize': Hello, World ~" << endl;
    //cout << "Rank: " << rank << ". Size/Numtasks: " << numtasks << endl;
    MPI_Get_processor_name(hostname, &len);
    printf("Size/Numtasks= %d, Rank= %d, Running on host %s, Len of hostname %d\n", numtasks,rank,hostname,len);

    // done with MPI
    MPI_Finalize();

    cout << "After 'MPI_Finalize': ---Hello, World!!!" << endl;
    return 0;
}