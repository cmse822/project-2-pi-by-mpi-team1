#include <iostream>
#include <mpi.h>

using namespace std;

int main(int argc, char *argv[]) 
{
    MPI_Init(&argc, &argv);

    int rank;
    int size;
    int name_length = MPI_MAX_PROCESSOR_NAME;
    char proc_name[name_length];

    MPI_Get_processor_name(proc_name,&name_length);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    cout << "Processor name: " << proc_name << endl; 
    cout << "Hello, World!" << endl;
    cout << "Rank: " << rank << ". Size: " << size << endl;

    MPI_Finalize();

    return 0;
}