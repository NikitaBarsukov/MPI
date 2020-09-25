#ifndef PROCESS_TASKS
#define PROCESS_TASKS
#include <mpi.h>
#include <string>

void sendMessage(const std::string &message, const int dest, int tag, MPI_Comm comm);

void receiveMessage(std::string &message, const int src, int tag, MPI_Comm comm);

void firstProcessTask(const unsigned int units_number);

void otherProcessTask(const int rank);

#endif
