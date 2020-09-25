#ifndef PROCESS_TASKS
#define PROCESS_TASKS
#include <mpi.h>
#include <string>

void packAndSend(const std::string &message, const int dest, const int tag);

void receiveAndUnpack(std::string &message, const int src, const int tag);

void firstProcessTask(const unsigned int units_number);

void otherProcessTask(const int rank);

#endif
