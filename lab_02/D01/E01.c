#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define NUM_THREADS 2

sem_t sem_read, sem_write, sem_ready, sem_count;
int g, c;

void thread_routine(void *args)
{
    int r, v, id = (long) args;
    printf("Thread #%d created\n", id);

    int f1, f2;

    switch(id)
    {
        case 1:
            f1 = open("fv1.b", O_RDONLY);
            if (!f1)
            {
                perror("Failed in opening fv1.b");
                exit(-1);
            }
            while (read(f1, &v, sizeof(int)))
            {
                printf("[Thread #%d] Value read = %d\n", id, v);
                fflush(stdout);
                sem_wait(&sem_write);
                g = v;
                sem_post(&sem_read);
                sem_wait(&sem_ready);
                printf("[Thread #%d] New value = %d\n", id, g);
                sem_post(&sem_write);

                sleep(3);
            }
            break;
        case 2:
            f2 = open("fv2.b", O_RDONLY);
            if (!f2)
            {
                perror("Failed in opening fv2.b");
                exit(-1);
            }
            while (read(f2, &v, sizeof(int)))
            {
                printf("[Thread #%d] Value read = %d\n", id, v);
                fflush(stdout);
                sem_wait(&sem_write);
                g = v;
                sem_post(&sem_read);
                sem_wait(&sem_ready);
                printf("[Thread #%d] New value = %d\n", id, g);
                sem_post(&sem_write);

                sleep(3);
            }
            break;
    }

    sem_wait(&sem_count);
    ++c;
    sem_post(&sem_count);
    printf("Thread #%d finished\n", id);
    pthread_exit(NULL);
}

int main(int argc, char *argv[], char *env[])
{

    int r;

    sem_init(&sem_ready, 0, 0);
    sem_init(&sem_read, 0, 0);
    sem_init(&sem_write, 0, 1);
    sem_init(&sem_count, 0, 1);
    printf("Semaphores initialized\n");

    pthread_t t[2];
    long i;
    for (i=0; i<NUM_THREADS; ++i){
        pthread_create(&t[i], NULL, (void *) thread_routine, (void*) i+1);
    }

    while (1)
    {

        printf("Threads that finished: %d\n", c);

        if (c == NUM_THREADS)
            break;

        r = sem_wait(&sem_read);
        if (r)
        {
            perror(strerror(errno));
            exit(-1);
        }
        g = g*3;
        r = sem_post(&sem_ready);
        if (r)
        {
            perror(strerror(errno));
            exit(-1);
        }
    }

    for (i=0; i<NUM_THREADS; ++i){
        pthread_join(t[i], NULL);
    }

}