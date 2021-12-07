#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define SIZE 200

/////////////////////////////
#include <pthread.h>
#include <iostream>
#include <semaphore.h>
#include <assert.h>
#include <queue>
#include <vector>
#include <tuple>

using namespace std;

pthread_t clientthreads[SIZE];
pthread_mutex_t outputlock;

struct clients {
    string command; 
    int client_id;
    int sleeptime;
    int fd;     // socket fd
};

typedef struct clients Clients;
