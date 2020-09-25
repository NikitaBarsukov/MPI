#include "split_encrypt.h"
#include "process_tasks.h"
#include <mpi.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank = 0, size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (rank == 0) {
        firstProcessTask(static_cast<unsigned int>(size));
    }
    else {
        otherProcessTask(rank);
    }
    MPI_Finalize();
    return 0;
}
