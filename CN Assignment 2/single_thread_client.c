#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int request_num) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char message[] = "Requesting top 2 processes";
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error in creating socket.\n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid Address error.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection error.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    send(sock, message, strlen(message), 0);
    printf("Request %d -> Top 2 processes Request Sent\n", request_num);

    read(sock, buffer, sizeof(buffer));
    printf("Request %d -> Message received:\n%s\n", request_num, buffer);

    close(sock);
}

void* client_thread(void* arg) {
    int request_num = *(int*)arg;
    handle_client(request_num);
    free(arg);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Not provided number of connection requests.\n");
        return 1;
    }

    int num_requests = atoi(argv[1]);
    if (num_requests <= 0) {
        printf("Invalid number of connection requests.\n");
        return 1;
    }

    // Handling multiple connection requests sequentially
    /*
    for (int i = 0; i < num_requests; i++) {
        handle_client(i + 1);
    }
    */

    // Multithreaded client connection requests
    pthread_t threads[num_requests];

    for (int i = 0; i < num_requests; i++) {
        int* request_num = malloc(sizeof(int));
        *request_num = i + 1;

        if (pthread_create(&threads[i], NULL, client_thread, (void*)request_num) != 0) {
            perror("Error creating thread");
            free(request_num);
            continue;
        }
    }

    for (int i = 0; i < num_requests; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
