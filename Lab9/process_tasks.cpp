#include "process_tasks.h"
#include "split_encrypt.h"
#include <mpi.h>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>

void sendMessage(const std::string &message, const int dest, const int tag, MPI_Comm comm) {
    const unsigned int length = message.length();
    //Сперва отправим размер сообщения
    MPI_Send(&length, 1, MPI_UNSIGNED, dest, tag, comm);
    if (length != 0) { //Если сообщение не пустое, то отправляем, иначе отправлять нечего
        MPI_Send(message.data(), length, MPI_CHAR, dest, tag, comm);
    }
}

void receiveMessage(std::string &message, const int src, const int tag, MPI_Comm comm) {
    unsigned int length = 0;
    MPI_Status status;
    //Получим размер сообщения, чтобы выделить для него место в векторе
    MPI_Recv(&length, 1, MPI_UNSIGNED, src, tag, comm, &status);
    if (length != 0) { //Если сообщение не пустое, то получаем, иначе и принимать нечего
        std::vector<char> tmp(length);
        MPI_Recv(tmp.data(), length, MPI_CHAR, src, tag, comm, &status);
        message.assign(tmp.begin(), tmp.end());
    }
}

//Первый процесс (модуль) - пользовательский ввод, создание ключа, рассылка частей сообщения и шифрование одной части сообщения
void firstProcessTask(unsigned int units_number) {
    std::string message;
    std::cout << "Input message: ";
    std::getline(std::cin, message, '\n');
    unsigned int shift = 1; //Сдвиг для шифра Цезаря
    std::cout << "Input shift value: ";
    std::cin >> shift;
    /* По методичке:
    Ключ - подстановка, строка которой задаёт номер модуля */
    std::vector<unsigned int> key[2];
    for (size_t j = 0; j != units_number; ++j) {
        key[0].push_back(j); //Номер модуля
        key[1].push_back(j); //Номер части сообщения, которую шифрует моудль (потом перемешаем)
    }

    std::random_device rd;
    std::mt19937 g(rd());
    //Нижняя строка определяет (с помощью рандома), какому модулю достанется определённая часть сообщения
    std::shuffle(key[1].begin(), key[1].end(), g);

    //Сообщение делится на части в соответствии с количеством модулей
    std::vector<std::string> splitted_message = sptitMessageIntoParts(message, units_number);

    /*Отправляем каждому модулю кроме текущего свою часть сообщения согласно номеру из подстановки
    rank = i + 1 (номер процесса), tag = i + 1 (тэг сделаем таким же для индетификации)
    Также разошлём всем shift - параметр сдвига в шифре Цезаря*/
    for (size_t i = 0; i != units_number - 1; ++i) {
        sendMessage(splitted_message[key[1][i + 1]], i + 1, i + 1, MPI_COMM_WORLD);
        MPI_Send(&shift, 1, MPI_UNSIGNED, i + 1, i + 1, MPI_COMM_WORLD);
    }

    /*Первый модуль (процесс, у которого rank == 0) шифрует часть сообщения согласно номеру из ключа-подстановки
    Нижняя строка, элемент с индексом 0*/
    if (!message.empty()) {
        message = encryptCaesarCipher(splitted_message[key[1][0]], shift);
        std::cout << "Process 0. Encrypted piece: " << message << '\n';
    }
    else {
        std::cout << "Process 0. An empty message received\n";
    }
}

//Остальные процессы (модули) - получение и шифровка кусочка сообщения
void otherProcessTask(const int rank) {
    std::string message;
    receiveMessage(message, 0, rank, MPI_COMM_WORLD);
    unsigned int shift = 1;
    MPI_Status status;
    MPI_Recv(&shift, 1, MPI_UNSIGNED, 0, rank, MPI_COMM_WORLD, &status);
    if (!message.empty()) {
        //Если сообщение не пустое, каждый модуль шифрует свою часть
        message = encryptCaesarCipher(message, shift);
        std::cout << "Process " << rank << ". Encrypted piece: " << message << '\n';
    }
    else {
        std::cout << "Process " << rank << ". An empty message received\n";
    }
}
