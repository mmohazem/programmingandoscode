#ifndef SECURITY_H
#define SECURITY_H

#define PSK "mysecretkey"

void encrypt(char* data, int length);
void decrypt(char* data, int length);

#endif
