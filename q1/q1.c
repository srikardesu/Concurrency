#include "header.h"

int rng(int a, int b) { // [a, b]
	int dif = b - a + 1 ; 
	int rnd = rand() % dif ;
	return (a + rnd);
}

void * handlestudents(void * arg) {
    Students * student = (Students *)arg;
    // printf("Details of the student are as follows: \n");
    // printf("Student ID: %d\n",student->stud_id);
    // printf("Student calibre: %f\n",student->calibre);
    // printf("Student priority 0: %d\n",student->pri[0]);
    // printf("Student reg time: %d\n",student->reg_time);

    sleep(student->reg_time);                      // wait for the student to fill

    pthread_mutex_lock(studmutex+student->stud_id);
    printf(CYAN"Student %d has filled in preferences for course registration form.\n",student->stud_id);
    student->is_avail=1;                           // student available for allocation now
    //printf("Students with student id: %d is available rn.\n",student->stud_id);
    pthread_mutex_unlock(studmutex+student->stud_id);

    while(student->curr_pri<=2) {
        pthread_mutex_lock(studmutex+student->stud_id);
        // printf(GREENBG"Student %d went to sleep.\n",student->stud_id);
        if(courselist[student->pri[student->curr_pri]].is_avail==0)
        pthread_cond_wait(cond+student->stud_id,studmutex+student->stud_id);
        // printf(GREENBG"Student %d got signal.\n",student->stud_id);
        int c_id = student->pri[student->curr_pri];

        // if course removed
        if(student->assigned_course != student->pri[student->curr_pri]) {
            if(courselist[c_id].is_avail==1) {
               // pthread_mutex_lock(studmutex+student->stud_id);
                // pthread_cond_wait(cond+student->stud_id,studmutex+student->stud_id);
                if(student->curr_pri==2) {
                    student->is_avail=0;
                    student->is_done=1;
                    printf(HRED"Student %d got no course :(.\n",student->stud_id);
                    printf(HRED "Student %d exited simulation.\n",student->stud_id);
                    return NULL;
                }
                printf(YELLOW "Student %d has changed his current preference from %s (priority %d) to %s (priority %d)\n",student->stud_id,courselist[student->pri[student->curr_pri]].name_course,student->curr_pri+1,courselist[student->pri[student->curr_pri+1]].name_course,student->curr_pri+2);
                student->curr_pri++;
                student->is_avail=1;
                pthread_mutex_unlock(studmutex+student->stud_id);
            }
        }
        // if course allocated
        else {
           // pthread_mutex_lock(studmutex+student->stud_id);
            // pthread_cond_wait(cond+student->stud_id,studmutex+student->stud_id);
            int val = 100*(student->calibre*courselist[c_id].interest_quot);
            int check_val = rng(1,100);
            if(check_val <= val) {
                // accepted
                printf(YELLOW "Student %d has selected the course %s permanently\n",student->stud_id,courselist[c_id].name_course);
                student->is_avail=0;
                student->is_done=1;
                printf(HRED "Student %d got %s :) %d priority.\n",student->stud_id,courselist[student->pri[student->curr_pri]].name_course,student->curr_pri+1);
                printf(HRED "Student %d exited simulation.\n",student->stud_id);
                pthread_mutex_unlock(studmutex+student->stud_id);
                return NULL;
            }
            else {
                // withdrawn
                printf(YELLOW "Student %d has withdrawn from course %s\n",student->stud_id,courselist[c_id].name_course);
                if(student->curr_pri==2) {
                    student->is_avail=0;
                    student->is_done=1;
                    printf(HRED"Student %d got no course :(.\n",student->stud_id);
                    printf(HRED "Student %d exited simulation.\n",student->stud_id);
                    return NULL;
                }
                printf(YELLOW "Student %d has changed his current preference from %s (priority %d) to %s (priority %d)\n",student->stud_id,courselist[student->pri[student->curr_pri]].name_course,student->curr_pri+1,courselist[student->pri[student->curr_pri+1]].name_course,student->curr_pri+2);
                student->curr_pri++;
                student->is_avail=1; 
                student->assigned_course=-1;
                pthread_mutex_unlock(studmutex+student->stud_id);
            }
            // if(student->is_avail==0) break;
        }
    }
}

void * handlecourses(void * arg) {
    sleep(1);
    while(1) {
        Courses * course = (Courses *) arg;

        // course removal check
        int lablist[SIZE];
        for(int i=0;i<course->no_of_labs;i++) {
            lablist[i]=course->lab_list[i];
        }
        int flag=0;
        for(int i=0;i<course->no_of_labs;i++) {
            int labno = lablist[i];
            for(int j=0;j<labs_list[labno].no_of_TAs;j++) {
                if(TA_list[labno][j].numdone<labs_list[labno].max_allocation_TA) {
                    flag=1;
                }
            }
        }
        if(!flag) {
            pthread_mutex_lock(coursemutex+course->course_id);
            printf(HRED "Course %s does not have any TA mentors eligible and is removed from course offerings\n",course->name_course);
            course->is_avail=1;
            printf(HRED "Course %s has exited simulation.\n",course->name_course);
            for(int i=0;i<studlist[0].tot_studs;i++) {
                if(studlist[i].pri[studlist[i].curr_pri]==course->course_id) {
                    pthread_cond_signal(cond+i);
                }
            }
            pthread_mutex_unlock(coursemutex+course->course_id);
            return NULL;
        }

        // int flag2=0;
        // for(int i=0;i<studlist[0].tot_studs;i++) {
        //     if(studlist[i].is_done==0 || studlist[i].pri[studlist[i].curr_pri]==course->course_id) {
        //         flag2=1;
        //     }
        // }
        // if(!flag2) {
        //     printf("Course %s has no more interested students\n",course->name_course);
        //     course->is_avail=1;
        //     printf("Course %s has exited simulation.\n",course->name_course);
        //     return NULL;

        // }

        for(int i=0;i<course->no_of_labs;i++) {
            int labno = lablist[i];
            for(int j=0;j<labs_list[labno].no_of_TAs;j++) {
                if(TA_list[labno][j].is_avail==0 && TA_list[labno][j].numdone<labs_list[labno].max_allocation_TA) {
                    // assign this TA to the course
                    // pthread_mutex_lock(coursemutex+course->course_id);
                    pthread_mutex_lock(&TAmutex[labno][j]);
                    TA_list[labno][j].is_avail=1;
                    TA_list[labno][j].numdone++;
                    printf(MAG "TA %d from lab %s has been allocated to course %s for his %dth TA ship.\n",j,labs_list[labno].name_lab,course->name_course,TA_list[labno][j].numdone);
                    int D = rng(1,course->course_max);
                    printf(MAG "Course %s has been allocated %d seats\n",course->name_course,D);
                    int stu=0; int studarr[SIZE]; int start = 0; int temp=0;
                    // int flag2=0;
                    // for(int i=0;i<studlist[0].tot_studs;i++) {
                    //     if(studlist[i].is_done==0 || studlist[i].pri[studlist[i].curr_pri]==course->course_id) {
                    //         flag2=1;
                    //     }
                    // }
                    // if(!flag2) {
                    //     printf("Course %s has no more interested students\n",course->name_course);
                    //     course->is_avail=1;
                    //     printf("Course %s has exited simulation.\n",course->name_course);
                    //     return NULL;

                    // }
                    // while(stu==0) {
                        //temp++; 
                        //printf(GREENBG "Course id: %d\n",course->course_id);
                        int cid = course->course_id;
                        for(int i=0;i<studlist[0].tot_studs;i++) {
                        //printf("debug Course_id: %d Course_name: %s Studid: %d assigned_course: %d priority_course: %d curr_pri: %d isavail?: %d studsalloc: %d\n",course->course_id,course->name_course,i,studlist[i].assigned_course,studlist[i].pri[studlist[i].curr_pri],studlist[i].curr_pri,studlist[i].is_avail,stu);
                            if(stu < D && studlist[i].is_done==0) {
                                if(studlist[i].is_avail==1 && studlist[i].assigned_course!=cid && studlist[i].pri[studlist[i].curr_pri]==cid) {
                                    //printf("ye\n");

                                    // assign students
                                    pthread_mutex_lock(studmutex+i);
                                    printf(GREENBG"Student %d has been allocated a seat in course %s\n",i,course->name_course);
                                    studlist[i].assigned_course=cid;
                                    stu++;
                                    studarr[start]=i;
                                    start++;
                                }
                            }
                        }
                    //     sleep(1);
                    // }
                    // if(temp==5 && stu==0) {
                    //     printf(HRED"Course %s has no more interested students\n",course->name_course);
                    //     course->is_avail=1;
                    //     printf(HRED"Course %s has exited simulation.\n",course->name_course);
                    //     for(int i=0;i<studlist[0].tot_studs;i++) {
                    //         if(studlist[i].pri[studlist[i].curr_pri]==course->course_id) {
                    //             pthread_cond_signal(cond+i);
                    //         }
                    //     }
                    //     return NULL;
                    // }
                    // sleep(1);
                    //pthread_mutex_lock(coursemutex+course->course_id);

                    // tutorial started
                    
                    printf(GREENBG"Tutorial has started for course %s with %d seats filled out of %d\n",course->name_course,stu,D);
                    sleep(3);              // change to sleep(1) if the tuts need to happen when atleast 1 student is to be alloted.
                    printf("TA %d from lab %s has completed the tutorial for course %s\n",j,labs_list[labno].name_lab,course->name_course);
                    //pthread_mutex_unlock(coursemutex+course->course_id);
                    for(int i=0;i<start;i++) {
                       studlist[studarr[i]].is_avail=0;
                       pthread_mutex_unlock(&studmutex[studarr[i]]);
                       pthread_cond_signal(cond+studarr[i]);
                    }

                    // lab removal
                    TA_list[labno][j].is_avail=0;
                    int fl=0;
                    for(int j=0;j<labs_list[labno].no_of_TAs;j++) {
                        if(TA_list[labno][j].numdone<labs_list[labno].max_allocation_TA && labs_list[labno].is_done==0) {
                            fl=1; break;
                        }
                    }
                    pthread_mutex_lock(labmutex+labs_list[labno].lab_id);
                    if(fl==0) {
                        if(labs_list[labno].is_done==0) {
                            printf(HRED"Lab %s no longer has students available for TA ship.\n",labs_list[labno].name_lab);
                            labs_list[labno].is_done=1;
                        }
                    }
                    pthread_mutex_unlock(labmutex+labs_list[labno].lab_id);
                    pthread_mutex_unlock(&TAmutex[labno][j]);
                    // pthread_mutex_unlock(coursemutex+course->course_id);
                }
            }
        }
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    int num_of_studs, num_of_labs, num_of_courses;
    scanf("%d %d %d",&num_of_studs,&num_of_labs,&num_of_courses);
    
    for(int i=0;i<num_of_studs;i++) {
        if (pthread_cond_init(cond+i, NULL) != 0) {                                    
            perror("pthread_cond_init() error");                                        
            exit(1);                                                                    
        }
    }

    for(int i=0;i<num_of_courses;i++) {
        courselist[i].course_id=i;
        scanf("%s",courselist[i].name_course);
        scanf("%f",&courselist[i].interest_quot);
        scanf("%d",&courselist[i].course_max);
        scanf("%d",&courselist[i].no_of_labs);
        for(int j=0;j<courselist[i].no_of_labs;j++) {
            scanf("%d",&courselist[i].lab_list[j]);
        }
        courselist[i].is_avail=0;
        courselist[i].tot_courses=num_of_courses;
    }

    for(int i=0;i<num_of_studs;i++) {
        studlist[i].stud_id=i;
        scanf("%f",&studlist[i].calibre);
        scanf("%d",&studlist[i].pri[0]);
        scanf("%d",&studlist[i].pri[1]);
        scanf("%d",&studlist[i].pri[2]);
        scanf("%d",&studlist[i].reg_time);
        studlist[i].is_avail=0;
        studlist[i].assigned_course=-1;
        studlist[i].curr_pri=0;
        studlist[i].tot_studs=num_of_studs;
        studlist[i].is_done=0;
    }


    for(int i=0;i<num_of_labs;i++) {
        labs_list[i].lab_id=i;
        scanf("%s",labs_list[i].name_lab);
        scanf("%d",&labs_list[i].no_of_TAs);
        scanf("%d",&labs_list[i].max_allocation_TA);
        for(int j=0;j<labs_list[i].no_of_TAs;j++) {
            TA_list[i][j].TA_id=j;
            TA_list[i][j].lab_id=i;
            TA_list[i][j].is_avail=0;
            TA_list[i][j].numdone=0;
        }
        labs_list[i].tot_labs=num_of_labs;
        labs_list[i].is_done=0;
    } 

    for(int i=0;i<num_of_labs;i++) {
        for(int j=0;j<labs_list[i].no_of_TAs;j++) {
            pthread_mutex_init(&TAmutex[i][j],NULL);
        }
    }

    for(int i=0;i<num_of_studs;i++) {
        pthread_create(&studentsthreads[i],NULL,handlestudents,studlist + i);
        pthread_mutex_init(studmutex+i,NULL);
    }
    for(int i=0;i<num_of_courses;i++) {
        pthread_create(&coursethreads[i],NULL,handlecourses,courselist + i);
        pthread_mutex_init(coursemutex+i,NULL);
    }
    for(int i=0;i<num_of_labs;i++) {
       pthread_mutex_init(labmutex+i,NULL);
    }

    for(int i=0;i<num_of_studs;i++) {
        pthread_join(studentsthreads[i],NULL);
    }
    // for(int i=0;i<num_of_courses;i++) {
    //     pthread_join(coursethreads[i],NULL);
    // }
}