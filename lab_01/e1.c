#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#ifndef DEBUG
#define DEBUG 1
#endif

void swap(int *xp, int *yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void bubble_sort(int *v, int n)
{
    int i, j;
    for (i=0; i<n; ++i)
    {
        for (j=0; j<n-i-1; ++j)
        {
            if (v[j] > v[j+1])
                swap(&v[j], &v[j+1]);
        }
    }
}

void print_vector(const char *name, int *v, int n)
{
    printf("[%s] ", name);
    int i;
    for (i=0; i<n; ++i)
    {
        printf("%d ", v[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[], char** env)
{   
    if (argc != 3)
    {
        fprintf(stderr, "[Usage] ./e1 n1 n2\n");
        exit(-1);
    }

    time_t t;

    int n1, n2;
    n1 = atoi(argv[1]);
    n2 = atoi(argv[2]);
    if (DEBUG)
        printf("n1 = %d, n2 = %d\n", n1, n2);

    int v1[n1];
    int v2[n2];

    srand((unsigned) time(&t));

    int i, r;
    for (i=0; i<n1; ++i)
    {
        r = (5 + (rand() % 46)) * 2;
        v1[i] = r;
    }
    for (i=0; i<n2; ++i)
    {
        r = ((10 + (rand() % 41)) * 2) + 1;
        v2[i] = r;
    }

    bubble_sort(v1, n1);
    bubble_sort(v2, n2);

    if (DEBUG)
    {
        print_vector("v1", v1, n1);
        print_vector("v2", v2, n2);
    }

    char buf[12];

    // fv1.txt
    FILE *fpw1;
    if ((fpw1 = fopen("fv1.txt", "w")) == NULL)
    {
        fprintf(stderr, "error in opening fv1.txt\n");
        exit(-1);
    }
    for (i=0; i<n1; ++i)
    {
        snprintf(buf, 12, "%d", v1[i]);
        fprintf(fpw1, "%s ", buf);
    }
    fprintf(fpw1, "\n");

    // fv2.txt
    FILE *fpw2;
    if ((fpw2 = fopen("fv2.txt", "w")) == NULL)
    {
        fprintf(stderr, "error in opening fv2.txt\n");
        exit(-1);
    }
    for (i=0; i<n2; ++i)
    {
        snprintf(buf, 12, "%d", v2[i]);
        fprintf(fpw2, "%s ", buf);
    }
    fprintf(fpw2, "\n");

    // fv1.b
    FILE *fpb1;
    if ((fpb1 = fopen("fv1.b", "wb")) == NULL)
    {
        fprintf(stderr, "error in opening fv1.b\n");
        exit(-1);
    }
    fwrite(v1, sizeof(v1), 1, fpb1);

    // fv2.b
    FILE *fpb2;
    if ((fpb2 = fopen("fv2.b", "wb")) == NULL)
    {
        fprintf(stderr, "error in opening fv2.b\n");
        exit(-1);
    }
    fwrite(v2, sizeof(v2), 1, fpb2);

    return 0;
}