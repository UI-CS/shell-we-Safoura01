#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <time.h>

#define MAX_PROCS 64

typedef struct
{
    long results[MAX_PROCS];
} shared_data;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <num_processes> <total_points>\n", argv[0]);
        return 1;
    }

    int num_processes = atoi(argv[1]);
    if (num_processes > MAX_PROCS)
        num_processes = MAX_PROCS;

    long total_points = atol(argv[2]);
    long points_per_process = total_points / num_processes;

    shared_data *data = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    for (int i = 0; i < num_processes; i++)
    {
        if (fork() == 0)
        {
            long count = 0;
            unsigned int seed = time(NULL) ^ (getpid() << 16);

            for (long j = 0; j < points_per_process; j++)
            {
                double x = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;
                double y = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;
                if (x * x + y * y <= 1.0)
                    count++;
            }
            data->results[i] = count;
            exit(0);
        }
    }

    for (int i = 0; i < num_processes; i++)
        wait(NULL);

    long total_inside = 0;
    for (int i = 0; i < num_processes; i++)
        total_inside += data->results[i];

    double pi = 4.0 * total_inside / (points_per_process * num_processes);
    printf("Estimated Pi = %f (Points: %ld)\n", pi, points_per_process * num_processes);

    munmap(data, sizeof(shared_data));
    return 0;
}