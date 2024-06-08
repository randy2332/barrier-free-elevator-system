#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "sockop.h"
#include "type.h"

#define SERVICE "tcp"
#define BUFFER_SIZE 1024

int shmid[32], Bitmap_id, timer_id, shm_fd, user_id;
int controller_pid, server_fd, client_fd;

Requirement *shm_data[32];
bitmap *shm_bitmap;

typedef struct {
    pthread_mutex_t mutex_child;
    pthread_mutex_t mutex_client;
    pthread_cond_t cond;
} share_mut_cond;

share_mut_cond *shm_mut_cond;

typedef struct {
    unsigned short data_time[32];
    char name[32][10];
} user;
user *user_info;

//sigint handler
void signalHandler(int signum) 
{
    for (int i = 0; i < 32; i++) {
        shmdt(shm_data[i]);
        shmctl(shmid[i], IPC_RMID, NULL);
    }
    shmdt(shm_bitmap);
    shmctl(Bitmap_id, IPC_RMID, NULL);
    shmctl(timer_id, IPC_RMID, NULL);
    shmdt(user_info);
    shmctl(user_id, IPC_RMID, NULL);
    munmap(shm_mut_cond, sizeof(share_mut_cond));
    close(shm_fd);
    if (client_fd != -1) {
        close(client_fd);
    }
    exit(signum);
}

int main(int argc, char *argv[]) 
{
    int data_index = 0;
    controller_pid = atoi(argv[1]);
    signal(SIGINT, signalHandler);
    char ip[20],port[20];
    strcpy(ip, argv[1]);
    strcpy(port, argv[2]);

    if (argc != 3) {
        fprintf(stderr, "Usage: %s [ip] [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    int server_fd = connectsock(ip, port, SERVICE);
    if (server_fd < 0) {
        perror("connectsock");
        exit(EXIT_FAILURE);
    }

    Requirement data[64];
    key_t key_data[32], key_bitmap = 1111, key_timer = 4444, key_userinfo = 5555;
    user_id = shmget(key_userinfo, sizeof(user), IPC_CREAT | 0666);
    user_info = (user *)shmat(user_id, NULL, 0);
    shm_fd = shm_open("/sharememory", O_RDWR, 0666);
    shm_mut_cond = mmap(NULL, sizeof(share_mut_cond), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    for (int i = 1; i <= 32; i++) {
        key_data[i - 1] = i;
        shmid[i - 1] = shmget(key_data[i - 1], sizeof(Requirement), IPC_CREAT | 0666);
        shm_data[i - 1] = (Requirement *)shmat(shmid[i - 1], NULL, 0);
    }

    Bitmap_id = shmget(key_bitmap, sizeof(bitmap), IPC_CREAT | 0666);
    shm_bitmap = (bitmap *)shmat(Bitmap_id, NULL, 0);

    char buffer[BUFFER_SIZE];
    char name[10];
    int type, time, loc, des;
    char direc[4];

INPUT:
    // Get user input
    printf("Enter data in the format: name <name> type <type> time <time> loc <loc> des <des> direc <up/down>\n");
    scanf("name %s type %d time %d loc %d des %d direc %s", name, &type, &time, &loc, &des, direc);

    // Construct the message to send
    snprintf(buffer, sizeof(buffer), "name %s type %d time %d loc %d des %d direc %s", name, type, time, loc, des, direc);

    // Send the message to the server
    if (write(server_fd, buffer, strlen(buffer)) < 0) {
        perror("write");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Data sent to server: %s\n", buffer);
    // memset(buffer, 0, sizeof(buffer));

    // // Receive and display response from the server continuously
    // while (1) {
    //     int bytes_read = read(server_fd, buffer, BUFFER_SIZE);
    //     if (bytes_read < 0) {
    //         printf("<0\n");
    //         perror("read");
    //         close(server_fd);
    //         exit(EXIT_FAILURE);
    //     } else if (bytes_read == 0) {
    //         printf("=0\n");
    //         printf("Server closed the connection.\n");
    //         break;
    //     } else {
    //         buffer[bytes_read] = '\0'; // Null-terminate the string
    //         printf(">0\n");
    //         printf("Response from server: %s\n", buffer);
    //     }
    // }

    // while (1) 
    // {
    //     pthread_mutex_lock(&shm_mut_cond->mutex_client);
    //     /* wait controller signal */
    //     pthread_cond_wait(&shm_mut_cond->cond, &shm_mut_cond->mutex_client);
    //     /* receive controller signal */
    //     /* renew I Check bitmap and O to I bitmap */
    //     int i = 0;
    //     while (i < 32) {
    //         if (shm_bitmap->zero_to_one & 1 << i) {
    //             if (shm_data[i]->state == ARRIVED) {
    //                 shm_bitmap->zero_to_one ^= 1 << i;
    //                 shm_bitmap->one ^= 1 << i;
    //                 printf("[Info] Passenger: %s\n", user_info->name[i]);
    //                 printf("            Service: Leave the Elevator\n");
    //                 printf("            Floor: %hu\n", shm_data[i]->destination);
    //                 printf("-----------------------------------------\n");
    //                 goto INPUT;
    //             } else if (shm_data[i]->state == INSIDE) {
    //                 shm_bitmap->zero_to_one ^= 1 << i;
    //                 printf("[Info]  Passenger: %s\n", user_info->name[i]);
    //                 printf("            Service: Enter the Elevator\n");
    //                 printf("            Floor: %hu\n", shm_data[i]->start);
    //                 printf("-----------------------------------------\n");
    //             }
    //         }
    //         i++;
    //     }
    //     pthread_mutex_unlock(&shm_mut_cond->mutex_client);
    // }

    // Close the socket
    close(server_fd);

    return 0;
}
