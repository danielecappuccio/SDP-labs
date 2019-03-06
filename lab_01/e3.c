#include <stdio.h>
#include <pthread.h>
#include <ctype.h>

char next, this, last;

void processing_routine(void* args)
{
    this = toupper(this);
    pthread_exit(NULL);
}

void output_routine(void* args)
{
    printf("%c", last);
    fflush(stdout);
    pthread_exit(NULL);
}

int main()
{
    FILE *f;
    f = fopen("input.in", "r");
    if (f == NULL)
    {
        fprintf(stderr, "Error during file opening\n");
        return -1;
    }

    int iteration = 0;

    pthread_t tp, to;
    while ((next = getc(f)) != EOF)
    {
        iteration++;
        if (iteration > 1)
            pthread_create(&tp, NULL, (void*) processing_routine, NULL);
        if (iteration > 2)
            pthread_create(&to, NULL, (void*) output_routine, NULL);

        if (iteration > 1)
            pthread_join(tp, NULL);
        if (iteration > 2)
            pthread_join(to, NULL);

        last = this;
        this = next;
    }

    // Unrolling the last Processing and Output operations

    pthread_create(&tp, NULL, (void*) processing_routine, NULL);
    pthread_create(&to, NULL, (void*) output_routine, NULL);

    pthread_join(tp, NULL);
    pthread_join(to, NULL);

    last = this;

    pthread_create(&to, NULL, (void*) output_routine, NULL);
    pthread_join(to, NULL);

    printf("\n");

    return 0;
}