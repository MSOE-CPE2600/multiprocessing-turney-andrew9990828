/**
 * Author: biebera@msoe.edu <Andrew Bieber>
 * Class: CPE 2600 - Systems Programming
 * Lab 12: Multithreading + Multiprocessing - mandelmovie
 *
 * Description:
 *  Generates 50 Mandelbrot frames. The work is divided among
 *  multiple child processes (-p) and each mandel invocation uses
 *  multiple threads (-t).
 *
 * NOTE: ChatGPT (5.1) used for structuring. All code assembled by Andrew Bieber.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define TOTAL_FRAMES 50

static void show_help() {
    printf("Usage: ./mandelmovie -p num_processes -t num_threads\n");
}

int main(int argc, char *argv[]) {
    int num_processes = 1;
    int num_threads = 1;
    char c;

    while ((c = getopt(argc, argv, "p:t:h")) != -1) {
        switch(c) {
            case 'p': num_processes = atoi(optarg); break;
            case 't': num_threads  = atoi(optarg); break;
            case 'h': show_help(); exit(0);
            default : show_help(); exit(1);
        }
    }

    if (num_processes < 1) num_processes = 1;
    if (num_threads  < 1) num_threads  = 1;

    printf("mandelmovie: processes=%d threads=%d\n", num_processes, num_threads);

    char t_str[16];
    snprintf(t_str, sizeof(t_str), "%d", num_threads);

    // spawn num_processes workers
    for (int worker = 0; worker < num_processes; worker++) {
        pid_t pid = fork();

        if (pid == 0) {
            // Child handles frames worker, worker + p, worker + 2p, ...
            for (int f = worker; f < TOTAL_FRAMES; f += num_processes) {

                pid_t frame_pid = fork();

                if (frame_pid == 0) {
                    // Construct output filename
                    char outfile[64];
                    snprintf(outfile, sizeof(outfile),
                             "mandel_%02d.jpg", f);

                    execl("./mandel", "mandel",
                          "-t", t_str,
                          "-o", outfile,
                          (char*)NULL);

                    perror("execl failed");
                    exit(1);
                }

                // parent waits for frame process
                waitpid(frame_pid, NULL, 0);
            }

            exit(0);
        }
    }

    // parent waits for all workers
    for (int i = 0; i < num_processes; i++)
        wait(NULL);

    return 0;
}