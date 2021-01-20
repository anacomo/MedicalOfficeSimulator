#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// se va citi numarul de pacienti
// maximul de consulatii
// maximum burst time

const int max_time = 7;
const int p = 5;

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Please introduce all values");
        return 0;
    }

    int max_c = atoi(argv[1]);
    int max_b = atoi(argv[2]);
    srand(time(0));

    FILE *file;
    file = fopen("generate.txt", "w");

    int start_time = 0;
    int last_time = 0;
    for (int i = 0; i < p; i++)
    {
        start_time = last_time + 1;
        int nr_consultations = 1 + rand() % max_c;
        fprintf(file, "%d ", nr_consultations);
        for (int j = 0; j < nr_consultations; j++)
        {
            if (j == 0)
            {
                last_time = start_time + rand() % 4;
            }
            fprintf(file, "%d %d ", start_time, 1 + rand() % max_b);
            start_time += rand() % max_b;
        }
        fprintf(file, "\n");
    }

    return 0;
}