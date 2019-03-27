#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE     64
#define NUM_ITERATIONS  10000

#define ERROR_HANDLER(cond, msg) {  \
    do { if (cond) { fprintf(stderr, msg); exit(EXIT_FAILURE); } } while (0); }

typedef struct condition
{
    pthread_mutex_t lock;
    pthread_cond_t notempty;
    pthread_cond_t notfull;
    int count;
} condition_t;

condition_t c;
int buffer[BUFFER_SIZE];
int read_idx=0, write_idx=0;

void sema_init(condition_t *c)
{
    pthread_mutex_init(&c->lock, NULL);
    pthread_cond_init(&c->notempty, NULL);
    pthread_cond_init(&c->notfull, NULL);
    c->count = 0;
}

void sema_wait(condition_t *c, char flag)
{   
    pthread_mutex_lock(&c->lock);

    switch (flag)
    {
        case 'P':
            while (c->count == BUFFER_SIZE)
                pthread_cond_wait(&c->notfull, &c->lock);
            break;
        case 'C':
            while (c->count == 0)
                pthread_cond_wait(&c->notempty, &c->lock);
            break;
        default:
            break;
    }
}

void sema_post(condition_t *c, char flag)
{
    switch (flag)
    {
        case 'P':
            pthread_cond_signal(&c->notempty);
            break;
        case 'C':
            pthread_cond_signal(&c->notfull);
            break;
        default:
            break;
    }

    pthread_mutex_unlock(&c->lock);
}

void producer_routine(void *args)
{
    int i, r;
    for (i=0; i<NUM_ITERATIONS; ++i)
    {
        sema_wait(&c, 'P');
        printf("putting %d in buffer\n", i);
        buffer[read_idx] = i;
        read_idx = (read_idx + 1) % BUFFER_SIZE;
        c.count++;
        sema_post(&c, 'P');
    }
    pthread_exit(NULL);
}

void consumer_routine(void *args)
{
    int i, r, n;
    for (i=0; i<NUM_ITERATIONS; ++i)
    {
        sema_wait(&c, 'C');
        n = buffer[write_idx];
        printf("reading %d from buffer\n", n);
        write_idx = (write_idx + 1) % BUFFER_SIZE;
        c.count--;
        sema_post(&c, 'C');
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int r;

    sema_init(&c);

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