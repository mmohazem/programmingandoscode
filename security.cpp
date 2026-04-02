#include "security.h"

#define KEY 0xAA

void encrypt(char* data, int length) {
    for (int i = 0; i < length; i++) {
        data[i] ^= KEY;
    }
}

void decrypt(char* data, int length) {
    for (int i = 0; i < length; i++) {
        data[i] ^= KEY;
    }
}
