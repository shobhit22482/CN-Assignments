#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

void* client_task(void *client_arg) {
    int client_num = *(int*)client_arg;
    int sock = 0;
    struct sockaddr_in serv_addr;
    char message[] = "Requesting top 2 processes";
    char buffer[BUFFER_SIZE] = {0};
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error in creating socket.\n");
        free(client_arg);
        pthread_exit(NULL);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid Address error.\n");
        close(sock);
        free(client_arg);
        pthread_exit(NULL);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection error.\n");
        close(sock);
        free(client_arg);
        pthread_exit(NULL);
    }

    send(sock, message, strlen(message), 0);
    printf("Client %d -> Top 2 processes Request Sent\n", client_num);

    read(sock, buffer, sizeof(buffer));
    printf("Client %d -> Message recieved:\n%s\n", client_num, buffer);

    close(sock);
    free(client_arg);
    pthread_exit(NULL);
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

    // Handling single client connection request with infinite input loop
    /*
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char message[BUFFER_SIZE];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error.\n");
        return -1;
    }
    printf("Socket created successfully.\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address error.\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection error.\n");
        return -1;
    }
    printf("Connected to the server.\n");

    printf("Enter a message (or Ctrl+D to quit): ");
    while (fgets(message, BUFFER_SIZE, stdin)) {
        message[strcspn(message, "\n")] = 0;
        send(sock, message, strlen(message), 0);
        printf("Message sent to server.\n");

        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(sock, buffer, BUFFER_SIZE);
        if (valread > 0) {
            buffer[valread] = '\0';
            printf("Server replied:\n%s\n", buffer);
        }

        printf("Enter a message (or Ctrl+D to quit): ");
    }

    close(sock);
    printf("Connection closed.\n");
    */

    // Multithreaded client connection requests
    pthread_t threads[num_requests];

    for (int i = 0; i < num_requests; i++) {
        int *client_num = malloc(sizeof(int));
        *client_num = i + 1;

        if (pthread_create(&threads[i], NULL, client_task, (void *)client_num) != 0) {
            perror("Couldn't create client thread.");
        }
    }

    for (int i = 0; i < num_requests; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
