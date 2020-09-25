#include "split_encrypt.h"
#include <string>
#include <vector>
#include <cctype>
#include <iterator>
#include <algorithm>

std::vector<std::string> sptitMessageIntoParts(const std::string &message, const unsigned int parts_number) {
    const unsigned int part_length = message.length() / parts_number;
    std::vector<std::string> result;
    result.reserve(parts_number);
    for (size_t i = 0; i != parts_number; ++i) {
        result.push_back(message.substr(i * part_length, part_length));
    }
    //Последний модуль получит всю оставшуюся часть сообщения
    if (part_length * parts_number < message.length()) {
        result.back() += message.substr(part_length * parts_number);
    }
    return result;
}

std::string encryptCaesarCipher(const std::string &message, const unsigned int shift) {
    std::string result;
    std::transform(message.cbegin(), message.cend(), std::back_inserter(result), [shift](const char symbol) {
        if (std::isupper(static_cast<unsigned char>(symbol))) {
            return static_cast<char>(static_cast<int>(symbol + shift - 65) % 26 + 65);
        }
        if (std::islower(static_cast<unsigned char>(symbol))) {
            return static_cast<char>(static_cast<int>(symbol + shift - 97) % 26 + 97);
        }
        return symbol;
    });
    return result;
}
