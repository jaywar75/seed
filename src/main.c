// main.c
#include <stdio.h>
#include "encrypt.h"

int main(void) {
    char input[] = "Hello, World!";
    char output[100];
    int key = 3;

    encrypt(input, output, key);
    printf("Encrypted: %s\n", output);

    // Optionally, test decryption
    char decrypted[100];
    decrypt(output, decrypted, key);
    printf("Decrypted: %s\n", decrypted);

    return 0;
}