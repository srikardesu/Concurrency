#include "headerq2.h"

int spectatingtime;

int rng(int a, int b) { // [a, b]
	int dif = b - a + 1 ; 
	int rnd = rand() % dif ;
	return (a + rnd);
}

void * handlespectators(void * arg) {
    Spectator * spec = (Spectator *)arg;
    sleep(spec->specsleeptime);                             // sleep spectator
    printf(BHMAG "%s has reached the stadium\n",spec->specname);
    int whichteam = spec->supportingteam;
    pthread_mutex_lock(spectatorsmutex+spec->specid);

    sem_post(&sem);

    spec->is_avail=1;              // avail now

    struct timespec t;
    time_t T;
    time(&T);
    t.tv_sec=T+spec->patience;                                  // patience 

    // HANDLE THE NEUTRAL LAD DIFFERENTLY

    if(spec->supportingteam==2) {
        // printf(BHMAG "%s going to sleep.\n",spec->specname);
        pthread_cond_timedwait(spectatorwait+spec->specid,spectatorsmutex+spec->specid,&t);
        // printf(HRED "%s got signalled first time.\n",spec->specname);
        // allocated

        if(spec->isalloc==1) {  
            pthread_mutex_unlock(spectatorsmutex+spec->specid);               
            int allocdzone = spec->zoneno;
            struct timespec t2;
            time_t T2;
            time(&T2);
            t2.tv_sec=T2+spectatingtime;
            pthread_mutex_lock(spectatorsmutex+spec->specid);
            int x = pthread_cond_timedwait(spectatorwait+spec->specid,spectatorsmutex+spec->specid,&t2);
            
            if(x == ETIMEDOUT) {
                printf(HMAG "Person %s has watched the match for %d secs and is leaving.\n",spec->specname,spectatingtime);
                printf(HMAG "Person %s is waiting for their friends at the exit.\n",spec->specname);
                spec->is_avail=0;
                pthread_mutex_trylock(zonemutex+spec->zoneno);
                printf(CYAN "Decreasing curr capacity of zone %d\n",spec->zoneno);
                zones[spec->zoneno].curr_capacity--;
                pthread_mutex_unlock(zonemutex+spec->zoneno);
                pthread_mutex_unlock(spectatorsmutex+spec->specid);
                return NULL;
            }

            else {
                printf(HRED "Person %s is leaving as the game got over.\n",spec->specname,spectatingtime);
                printf(HRED "Person %s is waiting for their friends at the exit.\n",spec->specname);
                spec->is_avail=0;
                pthread_mutex_trylock(zonemutex+spec->zoneno);
                zones[spec->zoneno].curr_capacity--;
                pthread_mutex_unlock(zonemutex+spec->zoneno);
                pthread_mutex_unlock(spectatorsmutex+spec->specid);
                return NULL;
            }
        }

        // not allocated

        else {
            printf(HRED "Person %s couldn't get a seat.\n",spec->specname);
            printf(HRED "Person %s is waiting for their friends at the exit.\n",spec->specname);
            spec->is_avail=0;
            pthread_mutex_unlock(spectatorsmutex+spec->specid);
            return NULL;
        }
    }
    pthread_cond_timedwait(spectatorwait+spec->specid,spectatorsmutex+spec->specid,&t);

    // printf(HRED "%s got signalled.\n",spec->specname);

    if(teams[1-whichteam].goalsscored>=spec->rmax) {
        printf(HRED "Person %s is leaving due to the bad defensive performance of his team.\n",spec->specname,spectatingtime);
        printf(HRED "Person %s is waiting for their friends at the exit.\n",spec->specname);
        spec->is_avail=0;
        pthread_mutex_trylock(zonemutex+spec->zoneno);
        printf(CYAN "Decreasing curr capacity of zone %d\n",spec->zoneno);
        zones[spec->zoneno].curr_capacity--;
        pthread_mutex_unlock(zonemutex+spec->zoneno);
        pthread_mutex_unlock(spectatorsmutex+spec->specid);
        return NULL;
    }

    // if(teams[1-whichteam].goalsscored>=spec->rmax && spec->isalloc==0) {
    //     printf("Person %s is leaving due to the bad defensive performance of his team.\n",spec->specname,spectatingtime);
    //     printf("Person %s is waiting for their friends at the exit.\n",spec->specname);
    //     spec->is_avail=0;
    //     pthread_mutex_unlock(spectatorsmutex+spec->specid);
    //     return NULL;
    // }

    // allocated

    if(spec->isalloc==1) {     
        pthread_mutex_unlock(spectatorsmutex+spec->specid);           
        int allocdzone = spec->zoneno;
        struct timespec t2;
        time_t T2;
        time(&T2);
        t2.tv_sec=T2+spectatingtime;
        pthread_mutex_lock(spectatorsmutex+spec->specid);  

        // wait for X time or untill rage quit happens.
        int x = pthread_cond_timedwait(spectatorwait+spec->specid,spectatorsmutex+spec->specid,&t2);

        if(teams[1-whichteam].goalsscored >= spec->rmax) {
            printf(HRED "Person %s is leaving due to the bad defensive performance of his team.\n",spec->specname,spectatingtime);
            printf(HRED "Person %s is waiting for their friends at the exit.\n",spec->specname);
            spec->is_avail=0;
            pthread_mutex_trylock(zonemutex+spec->zoneno);
            printf(CYAN "Decreasing curr capacity of zone %d\n",spec->zoneno);
            zones[spec->zoneno].curr_capacity--;
            pthread_mutex_unlock(zonemutex+spec->zoneno);
            pthread_mutex_unlock(spectatorsmutex+spec->specid);
            return NULL;
        }

        // X secs
        if(x == ETIMEDOUT) {
            printf(HMAG "Person %s has watched the match for %d secs and is leaving.\n",spec->specname,spectatingtime);
            printf(HMAG "Person %s is waiting for their friends at the exit.\n",spec->specname);
            spec->is_avail=0;
            pthread_mutex_trylock(zonemutex+spec->zoneno);
            printf(CYAN "Decreasing curr capacity of zone %d\n",spec->zoneno);
            zones[spec->zoneno].curr_capacity--;
            pthread_mutex_unlock(zonemutex+spec->zoneno);
            pthread_mutex_unlock(spectatorsmutex+spec->specid);
            return NULL;
        }

        // rage quit
        else {
            printf(HRED "Person %s is leaving due to the bad defensive performance of his team.\n",spec->specname,spectatingtime);
            printf(HRED "Person %s is waiting for their friends at the exit.\n",spec->specname);
            spec->is_avail=0;
            pthread_mutex_trylock(zonemutex+spec->zoneno);
            printf(CYAN "Decreasing curr capacity of zone %d\n",spec->zoneno);
            zones[spec->zoneno].curr_capacity--;
            pthread_mutex_unlock(zonemutex+spec->zoneno);
            pthread_mutex_unlock(spectatorsmutex+spec->specid);
            return NULL;
        }
    }

    // not allocated

    else {
        printf(HRED "Person %s couldn't get a seat.\n",spec->specname);
        printf(HRED "Person %s is waiting for their friends at the exit.\n",spec->specname);
        spec->is_avail=0;
        pthread_mutex_unlock(spectatorsmutex+spec->specid);
        return NULL;
    }
    return NULL;
}

void * zoneallocater (void * arg) {
    Zone * currzone = (Zone *)arg;
    while(1) {
        // wait till atleast one spec is there
        sem_wait(&sem);
        if(currzone->zoneid == 0) {
            pthread_mutex_lock(zonemutex+currzone->zoneid);
            int spectatorsqueue[SIZE]; int start=0;
            for(int i=0;i<spectators[0].totspec;i++) {
                pthread_mutex_lock(spectatorsmutex+spectators[i].specid);
                if(zones[0].curr_capacity < zones[0].tot_capacity && spectators[i].is_avail==1 && spectators[i].isalloc==0 && (spectators[i].supportingteam==0 || spectators[i].supportingteam==2)) {
                    // allocating spectators
                    printf(YELLOW "%s has got a seat in Zone H\n",spectators[i].specname);
                    spectatorsqueue[start]=spectators[i].specid;
                    start++;
                    zones[0].curr_capacity++;
                    spectators[i].isalloc=1;
                    spectators[i].zoneno=0;
                }
                else {
                    pthread_mutex_unlock(spectatorsmutex+spectators[i].specid);
                }
            }
            // cond signals to patience cond timed wait
            pthread_mutex_unlock(zonemutex+currzone->zoneid);
            for(int i=0;i<start;i++) {
                pthread_mutex_unlock(spectatorsmutex+spectatorsqueue[i]);
                // printf(BHMAG "signalling %s rn.\n",spectators[spectatorsqueue[i]].specname);
                pthread_cond_signal(spectatorwait+spectatorsqueue[i]);
            }
        }
        else if(currzone->zoneid == 1) {
            pthread_mutex_lock(zonemutex+currzone->zoneid);
            int spectatorsqueue[SIZE]; int start=0;
            for(int i=0;i<spectators[0].totspec;i++) {
                pthread_mutex_lock(spectatorsmutex+spectators[i].specid);
                if(zones[1].curr_capacity < zones[1].tot_capacity && spectators[i].is_avail==1 && spectators[i].isalloc==0 && (spectators[i].supportingteam==1 || spectators[i].supportingteam==2)) {
                    printf(YELLOW "%s has got a seat in Zone A\n",spectators[i].specname);
                    spectatorsqueue[start++]=spectators[i].specid;
                    zones[1].curr_capacity++;
                    spectators[i].isalloc=1;
                    spectators[i].zoneno=1;
                }
                else {
                    pthread_mutex_unlock(spectatorsmutex+spectators[i].specid);
                }
            }
            pthread_mutex_unlock(zonemutex+currzone->zoneid);
            // cond signals to patience cond timed wait
            for(int i=0;i<start;i++) {
                pthread_mutex_unlock(spectatorsmutex+spectatorsqueue[i]);
                //printf(HRED "signalling %s rn.\n",spectators[spectatorsqueue[i]].specname);
                pthread_cond_signal(spectatorwait+spectatorsqueue[i]);
            }
        }
        else {
            pthread_mutex_lock(zonemutex+currzone->zoneid);
            int spectatorsqueue[SIZE]; int start=0;
            for(int i=0;i<spectators[0].totspec;i++) {
                pthread_mutex_lock(spectatorsmutex+spectators[i].specid);
                if(zones[2].curr_capacity < zones[2].tot_capacity && spectators[i].is_avail==1 && spectators[i].isalloc==0 && (spectators[i].supportingteam==0 || spectators[i].supportingteam==2)) {
                    printf(YELLOW "%s has got a seat in Zone N\n",spectators[i].specname);
                    zones[2].curr_capacity++;
                    spectatorsqueue[start++]=spectators[i].specid;
                    spectators[i].isalloc=1;
                    spectators[i].zoneno=2;
                }
                else {
                    pthread_mutex_unlock(spectatorsmutex+spectators[i].specid);
                }
            }
            // cond signals to patience cond timed wait
            pthread_mutex_unlock(zonemutex+currzone->zoneid);
            for(int i=0;i<start;i++) {
                pthread_mutex_unlock(spectatorsmutex+spectatorsqueue[i]);
                pthread_cond_signal(spectatorwait+spectatorsqueue[i]);
            }
        }
        // sleep(0.1);
    }
    return NULL;
}

void * matchongoing(void * arg) {
    Matchdetails * goal = (Matchdetails *)arg;
    sleep(goal->sleeptime);
    int team = goal->whichteam;
    float p = goal->successprob;
    int d = rng(1,100);
    int val = (int)(p*100);
    // printf(YELLOW "Random val for %d\n",d);
    if(d<=val) {
        if(team==0) {
            pthread_mutex_lock(&teammutex[0]);
            printf(GREENBG "Team H has scored their %d th goal.\n",teams[0].goalsscored+1);
            teams[0].goalsscored++;
            int spectatorsqueue[SIZE]; int start=0;
            for(int i=0;i<spectators[0].totspec;i++) {
                if(spectators[i].isalloc==1 && spectators[i].is_avail==1 && spectators[i].supportingteam==1 && spectators[i].rmax<=teams[0].goalsscored) {
                    pthread_mutex_lock(spectatorsmutex+i);
                    spectatorsqueue[start++]=i;
                }
            }

            // signal all the rage quit spectators
            for(int i=0;i<start;i++) {
                pthread_mutex_unlock(spectatorsmutex+spectatorsqueue[i]);
                pthread_cond_signal(spectatorwait+spectatorsqueue[i]);
            }
            pthread_mutex_unlock(&teammutex[0]);
        }
        else {
            pthread_mutex_lock(&teammutex[1]);
            printf(GREENBG "Team A has scored their %d th goal.\n",teams[1].goalsscored+1);
            teams[1].goalsscored++;
            int spectatorsqueue[SIZE]; int start=0;
            for(int i=0;i<spectators[0].totspec;i++) {
                if(spectators[i].isalloc==1 && spectators[i].is_avail==1 && spectators[i].supportingteam==0 && spectators[i].rmax<=teams[1].goalsscored) {
                    pthread_mutex_lock(spectatorsmutex+i);
                    spectatorsqueue[start++]=i;
                }
            }

            // signal all the rage quit spectators
            for(int i=0;i<start;i++) {
                pthread_mutex_unlock(spectatorsmutex+spectatorsqueue[i]);
                pthread_cond_signal(spectatorwait+spectatorsqueue[i]);
            }
            pthread_mutex_unlock(&teammutex[1]);
        }
    }
    else {
        if(team==0) {
            printf(MAG "Team H has missed their chance to score their %d th goal.\n",teams[0].goalsscored+1);
        }
        else {
            printf(MAG "Team A has missed their chance to score their %d th goal.\n",teams[1].goalsscored+1);
        }
    }
    // pthread_mutex_lock(teammutex+team);
    // int q[SIZE];
    // int st = 0;
    // if(goal->eventid==goal->maxid) {
    //     for(int i=0;i<spectators[0].totspec;i++) {
    //         if(spectators[i].isalloc==1) {
    //             pthread_mutex_lock(spectatorsmutex+i);
    //             q[st++]=i;
    //         }
    //     }
    // } 
    // for(int i=0;i<st;i++) {
    //     pthread_mutex_unlock(spectatorsmutex+q[i]);
    //     pthread_cond_signal(spectatorwait+q[i]);
    // }
    // pthread_mutex_unlock(teammutex+team);
    return NULL;
}

int main() {
    srand(time(NULL));
    int h,a,n; scanf("%d %d %d",&h,&a,&n);
    zones[0].zoneid=0;
    zones[0].tot_capacity=h;
    zones[0].curr_capacity=0;

    zones[1].zoneid=1;
    zones[1].tot_capacity=a;
    zones[1].curr_capacity=0;

    zones[2].zoneid=2;
    zones[2].tot_capacity=n;
    zones[2].curr_capacity=0;

    int x; scanf("%d",&x);
    spectatingtime=x;

    sem_init(&sem,0,0);

    int num_groups; scanf("%d",&num_groups);
    // printf("num_groups: %d\n",num_groups);
    int idspec=0;
    for(int i=0;i<num_groups;i++) {
        int ppl; scanf("%d",&ppl);
        // printf("num_people: %d\n",ppl);
        for(int j=0;j<ppl;j++) {
            char c;
            scanf("%s",spectators[idspec].specname);
            scanf(" %c ",&c);
            scanf("%d",&spectators[idspec].specsleeptime);
            scanf("%d",&spectators[idspec].patience);
            scanf("%d",&spectators[idspec].rmax);
            if(c=='H') spectators[idspec].supportingteam=0;
            else if(c=='A') spectators[idspec].supportingteam=1;
            else spectators[idspec].supportingteam=2;
            spectators[idspec].specid=idspec;
            spectators[idspec].groupid=i;
            spectators[idspec].isalloc=0;                    // not alloc'd
            spectators[idspec].zoneno=-1;                    // not alloc'd
            idspec++;
        }
    } 
    
    for(int i=0;i<idspec;i++) {
        spectators[i].is_avail=0;                    // not avail
        spectators[i].totspec=idspec;
    }

    teams[0].team_id=0;
    teams[0].goalsscored=0;
    teams[1].team_id=1;
    teams[1].goalsscored=0;

    int G; scanf("%d",&G);
    // printf("chances: %d\n",G);
    for(int i=0;i<G;i++) {
        goals[i].eventid=i;
        char c;
        scanf(" %c %d %f",&c,&goals[i].sleeptime,&goals[i].successprob);
        if(c=='H') goals[i].whichteam=0;
        else goals[i].whichteam=1;
        goals[i].maxid=G-1;
    }
    for(int i=0;i<2;i++) {
        pthread_mutex_init(&teammutex[i],NULL);
    }

    for(int i=0;i<idspec;i++) {
        pthread_mutex_init(spectatorsmutex+i,NULL);
    }

    for(int i=0;i<idspec;i++) {
        if (pthread_cond_init(spectatorwait+i, NULL) != 0) {                                    
            perror("pthread_cond_init() error");                                        
            exit(1);                                                                    
        }
    }

    for(int i=0;i<3;i++) {
        pthread_create(zonethreads+i,NULL,zoneallocater,zones+i);
        pthread_mutex_init(zonemutex+i,NULL);
    }
    for(int i=0;i<idspec;i++) {
        pthread_create(spectatorthreads+i,NULL,handlespectators,spectators+i);
    }
    for(int i=0;i<G;i++) {
        pthread_create(goalthreads+i,NULL,matchongoing,goals+i);          // no need mutex cuz same time 2 events cant happen?
    }

    for(int i=0;i<idspec;i++) {
        pthread_join(spectatorthreads[i],NULL);
    }

    // for(int i=0;i<G;i++) {
    //     pthread_join(goalthreads[i],NULL);
    // }
}



