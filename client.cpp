#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include "security.h"

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cout << "Socket creation failed" << std::endl;
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cout << "Invalid address" << std::endl;
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Connection failed" << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Sending PSK..." << std::endl;
    send(sock, PSK, strlen(PSK), 0);

    read(sock, buffer, BUFFER_SIZE);

    if (strcmp(buffer, "AUTH_OK") != 0) {
        std::cout << "Authentication failed" << std::endl;
        close(sock);
        return 0;
    }

    std::cout << "Authenticated successfully" << std::endl;

    char msg[] = "Hello from client";
    encrypt(msg, strlen(msg));

    std::cout << "Sending encrypted message..." << std::endl;
    send(sock, msg, strlen(msg), 0);

    memset(buffer, 0, BUFFER_SIZE);
    int valread = read(sock, buffer, BUFFER_SIZE);

    if (valread > 0) {
        std::cout << "Encrypted reply: " << buffer << std::endl;
        decrypt(buffer, valread);
        std::cout << "Decrypted server message: " << buffer << std::endl;
    }

    close(sock);
    return 0;
}
