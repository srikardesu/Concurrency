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
#define HRED "\e[0;91m"

struct courses {
    int course_id;                       // course id
    char name_course[SIZE];              // name of the course
    float interest_quot;                 // interest of that course 
    int course_max;                      // max students that can potentially be allocated by a TA
    int no_of_labs;                      // the number of labs for that course
    int lab_list[SIZE];                  // the list of labs for that course
    int is_avail;                        // 0 implies course is there, 1 implies its ded
    int tot_courses;                     // total courses isnt global so
};

struct students {
    int stud_id;                        // student id
    float calibre;                      // calibre of the student
    int pri[3];                         // list of priority courses.
    int curr_pri;                       // current priority course that studnet is aiming for, 0 initially
    int reg_time;                       // sleeptime 
    int is_avail;                       // shows the availability of the student, 1 when student is asleep or in tut.
    int assigned_course;                // allocated course number, -1 if nothing is allocated.
    int tot_studs;                      // total students isnt global so
    int is_done;                        // extra check for when the student leaves simulation
};

struct labs {
    int lab_id;                         // ID of the lab.
    char name_lab[SIZE];                // Lab name
    int no_of_TAs;                      // number of TAs in this lab
    int max_allocation_TA;              // max times a TA from this lab can TA a course.
    int tot_labs;                       // total labs isnt global so
    int is_done;                        // extra check, 1 when the lab leaves, 0 otherwise
};

struct TAs {
    int TA_id;                         // TA id of that Lab.
    int lab_id;                        // the lab id form which the TA is from.
    int tot_TAs;                       // total TAs isnt global so
    int is_avail;                      // to see if the TA is available or taking a tut.
    int numdone;                       // number of TAships done
};

typedef struct courses Courses;
typedef struct students Students;
typedef struct labs Labs;
typedef struct TAs TA;

void * handlestudents(void * arg);
void * handlecourses(void * arg);

pthread_cond_t cond[SIZE];

pthread_t coursethreads[SIZE];
pthread_t studentsthreads[SIZE];
pthread_t labsthreads[SIZE];

pthread_mutex_t studmutex[SIZE];
pthread_mutex_t coursemutex[SIZE];
pthread_mutex_t labmutex[SIZE];
pthread_mutex_t TAmutex[SIZE][SIZE];

Courses courselist[SIZE];
Students studlist[SIZE];
Labs labs_list[SIZE];
TA TA_list[SIZE][SIZE];        // lab no, TA no in that lab