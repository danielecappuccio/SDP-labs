#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#define NUM_ITERATIONS 10000
#define BUFFER_SIZE    100

#define ERROR_HANDLER(cond, msg) { \
    do { if (cond) { fprintf(stderr, msg); exit(EXIT_FAILURE); } } while (0); }

long long normal[BUFFER_SIZE];
long long urgent[BUFFER_SIZE];
int nidx=0, uidx=0;                 // producers
int njdx=0, ujdx=0;                 // consumers

sem_t normal_prod, normal_cons, urgent_prod, urgent_cons;

long long current_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

void producer_routine(void *args)
{
    long long ms;
    int i, r, m; time_t t;
    srand(time(&t));
    for (i=0; i<NUM_ITERATIONS; ++i)
    {
        m = (rand() % 10) + 1;
        usleep(m*1000);
        ms = current_timestamp();

        if (m < 9)
        {
            // normal buffer (80% of times)
            r = sem_wait(&normal_prod);
            ERROR_HANDLER(r != 0, "Error in sem_wait on normal_prod semaphore\n");

            normal[nidx] = ms; nidx = (nidx + 1) % BUFFER_SIZE;
            printf("putting %lld in buffer <normal>\n", ms);

            r = sem_post(&normal_cons);
            ERROR_HANDLER(r != 0, "Error in sem_post on normal_cons semaphore\n");
        }
        else
        {
            // urgent buffer (20% of times)
            r = sem_wait(&urgent_prod);
            ERROR_HANDLER(r != 0, "Error in sem_wait on urgent_prod semaphore\n");

            urgent[uidx] = ms; uidx = (uidx + 1) % BUFFER_SIZE;
            printf("putting %lld in buffer <urgent>\n", ms);

            r = sem_post(&urgent_cons);
            ERROR_HANDLER(r != 0, "Error in sem_post on urgent_cons semaphore\n");
        }
    }

    pthread_exit(NULL);
}

void consumer_routine(void *args)
{
    int i, r, m=10; long long ms;
    for (i=0; i<NUM_ITERATIONS; ++i)
    {
        usleep(m*1000);

        r = sem_trywait(&urgent_cons);

        if (r == 0)
        {
            ms = urgent[ujdx]; ujdx = (ujdx + 1) % BUFFER_SIZE;
            printf("reading %lld from buffer <urgent>\n", ms);

            r = sem_post(&urgent_prod);
            ERROR_HANDLER(r != 0, "Error in sem_post on urgent_prod semaphore\n");
        }

        else if (r == -1 && errno == EAGAIN)
        {
            r = sem_wait(&normal_cons);
            ERROR_HANDLER(r != 0, "Error in sem_wait on normal_cons semaphore\n");

            ms = normal[njdx]; njdx = (njdx + 1) % BUFFER_SIZE;
            printf("reading %lld from buffer <normal>\n", ms);

            r = sem_post(&normal_prod);
            ERROR_HANDLER(r != 0, "Error in sem_post on normal_prod semaphore\n");
        }

        else
            ERROR_HANDLER(1, "Error on sem_trywait on urgent_cons semaphore\n");
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int r;

    r = sem_init(&normal_prod, 0, BUFFER_SIZE);
    ERROR_HANDLER(r != 0, "Error in initializing normal_prod semaphore\n");
    r = sem_init(&normal_cons, 0, 0);
    ERROR_HANDLER(r != 0, "Error in initializing normal_cons semaphore\n");
    r = sem_init(&urgent_prod, 0, BUFFER_SIZE);
    ERROR_HANDLER(r != 0, "Error in initializing urgent_prod semaphore\n");
    r = sem_init(&urgent_cons, 0, 0);
    ERROR_HANDLER(r != 0, "Error in initializing urgent_cons semaphore\n");

    pthread_t thp, thc;
    r = pthread_create(&thp, NULL, (void*) producer_routine, NULL);
    ERROR_HANDLER(r != 0, "Error in creating producer thread\n");
    r = pthread_create(&thc, NULL, (void*) consumer_routine, NULL);
    ERROR_HANDLER(r != 0, "Error in creating consumer thread\n");

    r = pthread_join(thp, NULL);
    ERROR_HANDLER(r != 0, "Error in joining producer thread\n");
    r = pthread_join(thc, NULL);
    ERROR_HANDLER(r != 0, "Error in joining consumer thread\n");

    return EXIT_SUCCESS;
}