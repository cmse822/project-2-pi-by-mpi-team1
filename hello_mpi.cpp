#include <iostream>
#include <mpi.h>

using namespace std;

int main(int argc, char *argv[]) 
{
    cout << "Before the 'init': Hello, World!" << endl;

    MPI_Init(&argc, &argv);

    int rank;
    int size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    cout << "Between the 'init' and 'finalize': Hello, World!" << endl;
    cout << "Rank: " << rank << ". Size: " << size << endl;

    MPI_Finalize();

    cout << "After the 'finalize': Hello, World!" << endl;

    return 0;
}