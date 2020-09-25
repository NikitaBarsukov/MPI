#include <mpi.h>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank = 0, size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    std::srand(static_cast<unsigned int>(std::time(nullptr) + rank));
    const int value_send = std::rand() % 10;
    double total_time = 0.0;
    if (rank == 0) {
        std::vector<int> values;
        values.reserve(static_cast<size_t>(size));
        int value_receive = 0;
        MPI_Status status;
        const double start_time = MPI_Wtime();
        //Процесс 0 отправляет значение самому себе и сам же получает
        MPI_Sendrecv(&value_send, 1, MPI_INT, 0, 0, &value_receive, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        values.push_back(value_receive);
        //Процесс 0 получает значения от остальных процессов
        for (size_t i = 0; i != static_cast<size_t>(size - 1); ++i) {
            MPI_Recv(&value_receive, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            values.push_back(value_receive);
        }
        const double elapsed_time = MPI_Wtime() - start_time;
        MPI_Reduce(&elapsed_time, &total_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        for (size_t i = 0; i != values.size(); ++i) {
            std::cout << "Received value [" << i << "]: " << values[i] << '\n';
        }
        std::cout << "Execution time: " << total_time << '\n';
    }
    else {
        const double start_time = MPI_Wtime();
        MPI_Send(&value_send, 1, MPI_INT, 0, rank, MPI_COMM_WORLD);
        const double elapsed_time = MPI_Wtime() - start_time;
        //Суммируем время работы каждого процесса в переменную total_time на процессе 0
        MPI_Reduce(&elapsed_time, &total_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}
