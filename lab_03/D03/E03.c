#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>

#define filename "fv.b"

#define ERROR_HANDLER(cond, msg) {  \
    do { if (cond) { fprintf(stderr, msg); exit(EXIT_FAILURE); } } while (0); }

typedef struct pthread_args 
{
    int *v;
    int left; 
    int right;
} pthread_args_t;

int threshold;

void swap_elements(int *v, int i, int j)
{
    int tmp = v[i]; v[i] = v[j]; v[j] = tmp;
}

void quicksort(int *v, int left, int right)
{
    int i, j, x, tmp;
    if (left >= right) return;
    x = v[left];
    i = left-1;
    j = right+1;
    while (i < j)
    {
        while (v[--j] > x);
        while (v[++i] < x);
        if (i < j)
            swap_elements(v, i, j);
    }
    quicksort(v, left, j);
    quicksort(v, j+1, right);
}

void thread_routine(void *args)
{
    pthread_args_t *t = (pthread_args_t *)args;
    int left = t->left, right = t->right, *v=t->v;
    int i, j, x, tmp, r;

    if (right-left < threshold)
    {
        // Go back to sequential version
        quicksort(v, left, right);
        pthread_exit(NULL);
    }

    if (left >= right)
        pthread_exit(NULL);

    x = v[left];
    i = left-1;
    j = right+1;
    while (i < j)
    {
        while (v[--j] > x);
        while (v[++i] < x);
        if (i < j)
            swap_elements(v, i, j);
    }

    pthread_t thl, thr;
    pthread_args_t *thl_args = malloc(sizeof(pthread_args_t));
    pthread_args_t *thr_args = malloc(sizeof(pthread_args_t));
    thl_args->v = v; thl_args->left = left, thl_args->right = j;
    thr_args->v = v; thr_args->left = j+1; thr_args->right = right;

    r = pthread_create(&thl, NULL, (void *)thread_routine, (void *)thl_args);
    ERROR_HANDLER(r != 0, "Error in creating thread\n");
    r = pthread_create(&thr, NULL, (void *)thread_routine, (void *)thr_args);
    ERROR_HANDLER(r != 0, "Error in creating thread\n");

    r = pthread_join(thl, NULL);
    ERROR_HANDLER(r != 0, "Error in joining thread\n");
    r = pthread_join(thr, NULL);
    ERROR_HANDLER(r != 0, "Error in joining thread\n");
    free(thl_args);
    free(thr_args);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int r;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./E03 <threshold>\n");
        exit(EXIT_FAILURE);
    }

    threshold = atoi(argv[1]);

    int fd = open(filename, O_RDWR);
    ERROR_HANDLER(fd == -1, "Error in opening file\n");
    struct stat st;
    stat(filename, &st);
    size_t filesize = st.st_size;
    printf("filesize: %zu bytes\n", filesize);

    int *v = (int *)mmap(NULL, filesize, PROT_WRITE, MAP_SHARED, fd, 0);
    ERROR_HANDLER(v == MAP_FAILED, "Error in mapping file into memory\n");

    int i, n=filesize/sizeof(int);
    printf("Original file: ");
    for (i=0; i<n; ++i)
        printf("%d ", v[i]);
    printf("\n");

    pthread_t thr;
    pthread_args_t *thr_args = malloc(sizeof(pthread_args_t));
    thr_args->v = v; thr_args->left = 0; thr_args->right = n-1;

    r = pthread_create(&thr, NULL, (void *)thread_routine, (void *)thr_args);
    ERROR_HANDLER(r != 0, "Error in creating thread\n");

    r = pthread_join(thr, NULL);
    ERROR_HANDLER(r != 0, "Error in joining thread\n");

    free(thr_args);

    printf("Ordered file: ");
    for (i=0; i<n; ++i)
        printf("%d ", v[i]);
    printf("\n");

    r = munmap(v, filesize);
    ERROR_HANDLER(r != 0, "Error in unmapping file\n");
    close(fd);

    return EXIT_SUCCESS;
}