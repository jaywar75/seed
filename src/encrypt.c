#include "encrypt.h"
#include <string.h>

void encrypt(char *input, char *output, int key) {
    int len = strlen(input);
    for (int i = 0; i < len; i++) {
        output[i] = input[i] + key;  // Shift each character by key
    }
    output[len] = '\0';  // Null-terminate the string
}

void decrypt(char *input, char *output, int key) {
    int len = strlen(input);
    for (int i = 0; i < len; i++) {
        output[i] = input[i] - key;  // Reverse the shift
    }
    output[len] = '\0';  // Null-terminate the string
}