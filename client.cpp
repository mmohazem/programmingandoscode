#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdio>
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

    char username[50], password[50];
    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter password: ";
    std::cin >> password;

    char login_data[100] = {0};
    sprintf(login_data, "%s %s", username, password);

    encrypt(login_data, strlen(login_data));
    send(sock, login_data, strlen(login_data), 0);

    memset(buffer, 0, BUFFER_SIZE);
    int valread = read(sock, buffer, BUFFER_SIZE);

    if (valread <= 0) {
        std::cout << "No response from server" << std::endl;
        close(sock);
        return 1;
    }

    decrypt(buffer, valread);
    std::cout << "Server response: " << buffer << std::endl;

    if (strncmp(buffer, "AUTH_OK", 7) != 0) {
        std::cout << "Authentication failed" << std::endl;
        close(sock);
        return 0;
    }

    char command[BUFFER_SIZE] = {0};
    std::cin.ignore();

    std::cout << "Enter command: ";
    std::cin.getline(command, BUFFER_SIZE);

    encrypt(command, strlen(command));
    send(sock, command, strlen(command), 0);

    memset(buffer, 0, BUFFER_SIZE);
    valread = read(sock, buffer, BUFFER_SIZE);

    if (valread > 0) {
        decrypt(buffer, valread);
        std::cout << "Server says: " << buffer << std::endl;
    }

    close(sock);
    return 0;
}
