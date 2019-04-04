#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

float *v1, *v2, **mat, *v;
int k;

void thread_routine(void *args)
{
    long i = (long) args;

    float tmp = 0; int j = 0;
    for (j=0; j<k; ++j)
        tmp += mat[i][j] * v2[j];
    
    v[i] = tmp;
    pthread_exit(NULL);
}

void fill_vector(float *v, int k)
{
    int i;
    for (i=0; i<k; ++i)
    {
        float x = (float) ((rand() % 10001) - 5000)/10000;
        v[i] = x;
    }
}

int main(int argc, char *argv[])
{
    long i;

    if (argc != 2)
    {
        perror("Usage: ./E4 <k>");
        exit(EXIT_FAILURE);
    }

    k = atoi(argv[1]);

    v1 = malloc(k*sizeof(float));
    v2 = malloc(k*sizeof(float));
    mat = malloc(k*sizeof(float *));
    
    srand(time(NULL));
    fill_vector(v1, k);
    fill_vector(v2, k);
    for (i=0; i<k; ++i){
        mat[i] = malloc(k*sizeof(float));
        fill_vector(mat[i], k);
    }

    v = malloc(k*sizeof(float));

    pthread_t th[k];
    for (i=0; i<k; ++i)
        pthread_create(&th[i], NULL, (void *) thread_routine, (void *) i);
    
    for (i=0; i<k; ++i)
        pthread_join(th[i], NULL);

    float result = 0;
    for (i=0; i<k; ++i)
        result += v1[i] * v[i];

    printf("result: %f\n", result);
    for (i=0; i<k; ++i)
        free(mat[i]);

    free(v1);
    free(v2);
    free(mat);
    free(v);

    return EXIT_SUCCESS;
}