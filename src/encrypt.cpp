#include "encrypt.hpp"

std::string encrypt(const std::string &input, int key) {
    std::string output = input;
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = input[i] + key; // Simple Caesar cipher shift
    }
    return output;
}

std::string decrypt(const std::string &input, int key) {
    std::string output = input;
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = input[i] - key;
    }
    return output;
}