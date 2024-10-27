#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 1024

volatile sig_atomic_t running = 1;
int server_fd = 0;

void handle_sigint(int sig) {
    running = 0;
    printf("\nReceived Ctrl+C, shutting down the server gracefully.\n");
    shutdown(server_fd, SHUT_RDWR); // forcing accept() to stop
}

struct process_info {
    int pid;
    char name[256];
    unsigned long long user_cpu_time;
    unsigned long long kernel_cpu_time;
    unsigned long long cpu_time;  // user + kernel time in clock ticks
};

unsigned long long get_process_cpu_time(int pid, struct process_info *proc) {
    char stat_path[128];
    sprintf(stat_path, "/proc/%d/stat", pid);

    FILE *stat_file = fopen(stat_path, "r");
    if (!stat_file) {
        return 0;
    }

    char buffer[512];
    if (fgets(buffer, sizeof(buffer), stat_file) == NULL) {
        fclose(stat_file);
        return 0;
    }

    char *fields[52]; 
    char *tok = strtok(buffer, " ");
    int idx = 0;

    while (tok != NULL && idx < 52) {
        fields[idx++] = tok;
        tok = strtok(NULL, " ");
    }

    proc->pid = atoi(fields[0]);

    snprintf(proc->name, sizeof(proc->name), "%s", fields[1]);
    size_t name_len = strlen(proc->name);
    if (proc->name[0] == '(' && proc->name[name_len - 1] == ')') {
        proc->name[name_len - 1] = '\0';
        memmove(proc->name, proc->name + 1, name_len - 1);
    }

    proc->user_cpu_time = strtoul(fields[13], NULL, 10);
    proc->kernel_cpu_time = strtoul(fields[14], NULL, 10);

    proc->cpu_time = proc->user_cpu_time + proc->kernel_cpu_time;

    fclose(stat_file);
    return proc->cpu_time;
}

void get_top_two_processes(struct process_info *top_processes) {
    struct dirent *entry;
    DIR *proc_dir = opendir("/proc");

    if (!proc_dir) {
        perror("opendir /proc error.\n");
        return;
    }

    struct process_info p1;
    struct process_info p2;
    
    unsigned long long maxi1 = 0;
    unsigned long long maxi2 = 0;

    while ((entry = readdir(proc_dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            const char *pid_str = entry->d_name;
            int is_pid = 1;
            for (int i = 0; pid_str[i] != '\0'; i++) {
                if (!isdigit(pid_str[i])) {
                    is_pid = 0;
                    break;
                }
            }
            if (is_pid) {
                int pid = atoi(pid_str);
                struct process_info proc;
                proc.pid = pid;
                proc.cpu_time = get_process_cpu_time(pid, &proc);

                if (proc.cpu_time > 0) {
                    if (proc.cpu_time > maxi1) {
                        p2 = p1;
                        p1 = proc;
                        maxi2 = maxi1;
                        maxi1 = proc.cpu_time;
                    } else if (proc.cpu_time > maxi2) {
                        p2 = proc;
                        maxi2 = proc.cpu_time;
                    }
                }
            }
        }
    }

    closedir(proc_dir);

    top_processes[0] = p1;
    top_processes[1] = p2;
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    
    if (read(client_socket, buffer, BUFFER_SIZE) < 0) {
        perror("Read error.\n");
        close(client_socket);
        return;
    }
    else {
        printf("Received request from client: %s\n", buffer);
    }

    if (strcmp(buffer, "Requesting top 2 processes") == 0) {
        struct process_info top_processes[2];
        get_top_two_processes(top_processes);

        sprintf(buffer, "Process 1: PID=%d, Name=%s, CPU Time=%llu\n"
                        "Process 2: PID=%d, Name=%s, CPU Time=%llu\n",
                top_processes[0].pid, top_processes[0].name, top_processes[0].cpu_time,
                top_processes[1].pid, top_processes[1].name, top_processes[1].cpu_time);
        send(client_socket, buffer, strlen(buffer), 0);
    }
    else {
        sprintf(buffer, "Unsupported request");
        send(client_socket, buffer, strlen(buffer), 0);
    }

    // Close the socket for this client
    close(client_socket);
}

int main() {
    int new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    signal(SIGINT, handle_sigint);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error in creating socket.\n");
        exit(EXIT_FAILURE);
    }
    printf("Server Socket created successfully.\n");

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){1}, sizeof(int)) < 0) {
        perror("Setsockopt error.\n");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(PORT); 

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind error.\n");
        exit(EXIT_FAILURE);
    }
    printf("Server Socket binded successfully.\n");

    if (listen(server_fd, 3) < 0) {
        perror("Listen error.\n");
        exit(EXIT_FAILURE);
    }
    printf("Server listening for incoming connection requests.\n");

    while (running) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            if (!running) break;
            perror("Accept error.\n");
            continue;
        }

        printf("Connection accepted.\n");

        handle_client(new_socket);
    }

    close(server_fd);
    printf("Server Socket closed.\n");

    return 0;
}