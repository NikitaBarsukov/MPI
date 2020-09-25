#ifndef SPLIT_ENCRYPT_H
#define SPLIT_ENCRYPT_H
#include <string>
#include <vector>

std::vector<std::string> sptitMessageIntoParts(const std::string &message, const unsigned int parts_number);

std::string encryptCaesarCipher(const std::string &message, const unsigned int shift);

#endif
