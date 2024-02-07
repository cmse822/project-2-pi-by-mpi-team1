#include <iostream>
#include <mpi.h>
#include <sstream>
#include <fstream>

using namespace std;

int main(int argc, char *argv[]) 
{
    MPI_Init(&argc, &argv);

    int rank;
    int size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        cout << "Rank: " << rank << endl;
        cout << "Total number of processes: " << size << endl;
    }

    MPI_Finalize();

    return 0;
}