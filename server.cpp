#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <cstdio>
#include "security.h"

#define PORT 8080
#define BUFFER_SIZE 1024

int authenticate(char* username, char* password, char* role_out) {
    FILE* file = fopen("users.txt", "r");
    if (file == NULL) {
        std::cout << "Could not open users.txt" << std::endl;
        return 0;
    }

    char file_user[50], file_pass[50], file_role[50];

    while (fscanf(file, "%49s %49s %49s", file_user, file_pass, file_role) != EOF) {
        if (strcmp(username, file_user) == 0 && strcmp(password, file_pass) == 0) {
            strcpy(role_out, file_role);
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

void* handle_client(void* socket_desc) {
    int client_socket = *(int*)socket_desc;
    delete (int*)socket_desc;

    char buffer[BUFFER_SIZE] = {0};
    char username[50] = {0};
    char password[50] = {0};
    char role[50] = {0};

    int valread = read(client_socket, buffer, BUFFER_SIZE);

    if (valread <= 0) {
        close(client_socket);
        pthread_exit(NULL);
    }

    decrypt(buffer, valread);
    std::cout << "Received login data: " << buffer << std::endl;

    sscanf(buffer, "%49s %49s", username, password);

    if (!authenticate(username, password, role)) {
        std::cout << "Authentication failed" << std::endl;
        char fail_msg[] = "AUTH_FAIL";
        encrypt(fail_msg, strlen(fail_msg));
        send(client_socket, fail_msg, strlen(fail_msg), 0);
        close(client_socket);
        pthread_exit(NULL);
    }

    std::cout << "Client authenticated successfully as role: " << role << std::endl;

    char auth_msg[100] = {0};
    sprintf(auth_msg, "AUTH_OK %s", role);
    encrypt(auth_msg, strlen(auth_msg));
    send(client_socket, auth_msg, strlen(auth_msg), 0);

    memset(buffer, 0, BUFFER_SIZE);
    valread = read(client_socket, buffer, BUFFER_SIZE);

    if (valread > 0) {
        decrypt(buffer, valread);
        std::cout << "Decrypted command from client: " << buffer << std::endl;

        char response[BUFFER_SIZE] = {0};

        if (strcmp(role, "entry") == 0) {
            if (strcmp(buffer, "ls") == 0 || strncmp(buffer, "read", 4) == 0) {
                strcpy(response, "Command allowed for entry level user");
            } else {
                strcpy(response, "ACCESS DENIED: Entry level can only use ls and read");
            }
        }
        else if (strcmp(role, "medium") == 0) {
            if (strcmp(buffer, "ls") == 0 || strncmp(buffer, "read", 4) == 0 ||
                strncmp(buffer, "copy", 4) == 0 || strncmp(buffer, "edit", 4) == 0) {
                strcpy(response, "Command allowed for medium level user");
            } else {
                strcpy(response, "ACCESS DENIED: Medium level cannot delete files");
            }
        }
        else if (strcmp(role, "top") == 0) {
            strcpy(response, "Command allowed for top level user");
        }
        else {
            strcpy(response, "ACCESS DENIED: Unknown role");
        }

        encrypt(response, strlen(response));
        send(client_socket, response, strlen(response), 0);
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
