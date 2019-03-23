#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

sem_t S;

void th1_routine(void *args)
{
    int r, t;
    long tmax;
    tmax = (long) args;
    srand(time(0));
    t = (rand() % 5) + 1;

    struct timespec ts, ts_sem;
    ts.tv_sec = 0;
    ts.tv_nsec = t*1000000;
    nanosleep(&ts, NULL);

    printf("[th1] waiting on semaphore after %d milliseconds\n", t);
    printf("[th1] tmax = %ld milliseconds\n", tmax);

    long sec = tmax/1000;
    tmax = tmax - (sec*1000);

    ts_sem.tv_sec = time(NULL) + sec;
    ts_sem.tv_nsec = tmax*1000000;

    r = sem_timedwait(&S, &ts_sem);
    if (r == -1 && errno == ETIMEDOUT)
        printf("[th1] wait on semaphore S returned for timeout\n");
    else
        printf("[th1] wait returned normally\n");
    
    pthread_exit(NULL);
}

void th2_routine(void *args)
{
    int t;
    srand(time(0));
    t = (rand() % 10) + 1;

    struct timespec ts;
    ts.tv_sec = t;
    ts.tv_nsec = 0;
    nanosleep(&ts, NULL);

    printf("[th2] performing signal on semaphore S after %d milliseconds\n", t*1000);
    sem_post(&S);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        perror("please provide arg tmax");
        exit(-1);
    }

    sem_init(&S, 0, 0);

    long tmax = atoi(argv[1]);

    pthread_t th1, th2;
    pthread_create(&th1, NULL, (void *)th1_routine, (void *) tmax);
    pthread_create(&th2, NULL, (void *)th2_routine, NULL);
    pthread_join(th1, NULL);
    pthread_join(th2, NULL);

    return 0;
}