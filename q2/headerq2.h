#include <assert.h>
#include <errno.h>
#include <math.h>
#include <memory.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define SIZE 100

#define CYAN "\e[0;36m"
#define GREENBG "\033[0;32m"
#define reset "\e[0m"
#define YELLOW "\e[0;33m"
#define MAG "\e[0;35m"
#define HMAG "\e[0;95m"
#define BHMAG "\e[1;95m"
#define YELHB "\e[0;103m"
#define HRED  "\e[0;31m"

// convention
// 0 is H
// 1 is A
// 2 is N

struct spectator {
    int specid;
    int groupid;
    int specsleeptime;
    int patience;
    int rmax;
    int supportingteam;
    char specname[SIZE];
    int is_avail;                        // 0 while sleeping and 1 while awake
    int totspec;
    int isalloc;
    int zoneno;
};

struct zone {
    int zoneid;
    int tot_capacity;
    int curr_capacity;
};

struct team {
    int team_id;
    int goalsscored;
};

struct matchdetails {
    int eventid;
    int whichteam;
    int sleeptime;
    float successprob;
    int maxid;
};

typedef struct spectator Spectator;
typedef struct zone Zone;
typedef struct team Team;
typedef struct matchdetails Matchdetails;

Zone zones[3];
Spectator spectators[SIZE];
Matchdetails goals[SIZE];
Team teams[2];

pthread_t goalthreads[SIZE];
pthread_t spectatorthreads[SIZE];
pthread_t zonethreads[SIZE];

pthread_mutex_t goalsmutex[SIZE];
pthread_mutex_t spectatorsmutex[SIZE];
pthread_mutex_t zonemutex[SIZE];
pthread_mutex_t teammutex[SIZE];

pthread_cond_t spectatorwait[SIZE];

sem_t sem; 

void * handlespectators(void * arg);
void * zoneallocater(void * arg);
void * matchongoing (void * arg);
