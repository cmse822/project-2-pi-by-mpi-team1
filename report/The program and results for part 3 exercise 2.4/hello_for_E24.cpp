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

    std::stringstream ss;
    ss << "rank_" << rank << ".txt";
    std::string filename = ss.str();

    std::ofstream outfile(filename.c_str());

    if (outfile.is_open()) {
        outfile << "Hello, World. Rank: " << rank << ". Size: " << size << std::endl;
        outfile.close();
    } else {
        std::cerr << "Cannot open file: " << filename << std::endl;
    }

    MPI_Finalize();

    return 0;
}