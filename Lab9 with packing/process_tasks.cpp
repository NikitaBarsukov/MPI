#include "process_tasks.h"
#include "split_encrypt.h"
#include <mpi.h>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>

void packAndSend(const std::string &message, const int dest, const int tag) {
    int outbuf_size = 0;
    MPI_Pack_size(message.length(), MPI_CHAR, MPI_COMM_WORLD, &outbuf_size);
    std::vector<char> outbuf(outbuf_size);
    MPI_Send(&outbuf_size, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
    if (outbuf_size != 0) {
        int position = 0;
        MPI_Pack(message.data(), message.length(), MPI_CHAR, outbuf.data(), outbuf_size, &position, MPI_COMM_WORLD);
        MPI_Send(outbuf.data(), outbuf_size, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
    }
}

void receiveAndUnpack(std::string &message, const int src, const int tag) {
    MPI_Status status;
    int buf_size = 0;
    MPI_Recv(&buf_size, 1, MPI_INT, src, tag, MPI_COMM_WORLD, &status);
    if (buf_size != 0) {
        std::vector<char> inbuf(static_cast<size_t>(buf_size));
        MPI_Recv(inbuf.data(), inbuf.size(), MPI_CHAR, src, tag, MPI_COMM_WORLD, &status);
        int position = 0;
        std::vector<char> outbuf(static_cast<size_t>(buf_size));
        MPI_Unpack(inbuf.data(), inbuf.size(), &position, outbuf.data(), outbuf.size(), MPI_CHAR, MPI_COMM_WORLD);
        message.assign(outbuf.begin(), outbuf.end());
    }
}

/* Первый процесс (модуль) - пользовательский ввод, создание ключа, упаковка и рассылка частей сообщения,
а также шифрование одной части сообщения */
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

    /* Отправляем каждому модулю кроме текущего свою часть сообщения согласно номеру из подстановки
    rank = i + 1 (номер процесса), tag = i + 1 (тэг сделаем таким же для индетификации)

    Также разошлём всем shift - параметр сдвига в шифре Цезаря
    Неизвестно, какой модуль получил самую большую часть сообщения
    Поэтому будем каждый раз менять размер буфера, чтобы всегда передавать нужное количество символов */
    for (size_t i = 0; i != units_number - 1; ++i) {
        packAndSend(splitted_message[key[1][i + 1]], i + 1, i + 1);
        MPI_Send(&shift, 1, MPI_UNSIGNED, i + 1, i + 1, MPI_COMM_WORLD);
    }

    /* Первый модуль (процесс, у которого rank == 0) шифрует часть сообщения согласно номеру из ключа-подстановки
    Нижняя строка, элемент с индексом 0 */
    if (!message.empty()) {
        message = encryptCaesarCipher(splitted_message[key[1][0]], shift);
        std::cout << "Process 0. Encrypted piece: " << message << '\n';
    }
    else {
        std::cout << "Process 0. An empty message received\n";
    }
}

//Остальные процессы (модули) - получение, распаковка и шифровка кусочка сообщения
void otherProcessTask(const int rank) {
    std::string message;
    receiveAndUnpack(message, 0, rank);
    MPI_Status status;
    unsigned int shift = 1;
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
