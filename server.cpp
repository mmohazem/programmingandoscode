#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "security.h"

#define PORT 8080
#define BUFFER_SIZE 1024

void* handle_client(void* socket_desc) {
    int client_socket = *(int*)socket_desc;
    delete (int*)socket_desc;

    char buffer[BUFFER_SIZE] = {0};

    read(client_socket, buffer, BUFFER_SIZE);
    std::cout << "Received PSK: " << buffer << std::endl;

    if (strcmp(buffer, PSK) != 0) {
        std::cout << "Authentication failed" << std::endl;
        close(client_socket);
        pthread_exit(NULL);
    }

    std::cout << "Client authenticated successfully" << std::endl;
    send(client_socket, "AUTH_OK", 7, 0);

    memset(buffer, 0, BUFFER_SIZE);
    int valread = read(client_socket, buffer, BUFFER_SIZE);

    if (valread > 0) {
        std::cout << "Encrypted message from client: " << buffer << std::endl;
        decrypt(buffer, valread);
        std::cout << "Decrypted client message: " << buffer << std::endl;

        char msg[] = "Hello from server";
        encrypt(msg, strlen(msg));
        std::cout << "Sending encrypted message to client..." << std::endl;
        send(client_socket, msg, strlen(msg), 0);
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cout << "Socket creation failed" << std::endl;
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cout << "Bind failed" << std::endl;
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        std::cout << "Listen failed" << std::endl;
        close(server_fd);
        return 1;
    }

    std::cout << "Multi-threaded server listening on port 8080..." << std::endl;

    while (true) {
        int client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);

        if (client_socket < 0) {
            std::cout << "Accept failed" << std::endl;
            continue;
        }

        std::cout << "New client connected" << std::endl;

        pthread_t thread_id;
        int* new_sock = new int;
        *new_sock = client_socket;

        if (pthread_create(&thread_id, NULL, handle_client, (void*)new_sock) < 0) {
            std::cout << "Could not create thread" << std::endl;
            close(client_socket);
            delete new_sock;
            continue;
        }

        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}
