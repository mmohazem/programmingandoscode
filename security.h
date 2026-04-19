#ifndef SECURITY_H
#define SECURITY_H

#include <cstring>

#define KEY 0xAA

void encrypt(char* data, int length);
void decrypt(char* data, int length);

#endif
