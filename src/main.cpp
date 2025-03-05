#include <iostream>
#include "encrypt.hpp"
#include "gpu_app.hpp"

int main() {
    std::string original = "Hello, ApexSuite!";
    int key = 3;

    std::string encrypted = encrypt(original, key);
    std::string decrypted = decrypt(encrypted, key);

    std::cout << "Encrypted: " << encrypted << std::endl;
    std::cout << "Decrypted: " << decrypted << std::endl;

    // Run the Vulkan-based vector addition demo.
    runVectorAddition();

    return 0;
}