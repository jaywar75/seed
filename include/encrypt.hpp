#ifndef ENCRYPT_HPP
#define ENCRYPT_HPP

#include <string>

// Encrypt and decrypt functions using a simple Caesar cipher.
std::string encrypt(const std::string &input, int key);
std::string decrypt(const std::string &input, int key);

#endif // ENCRYPT_HPP