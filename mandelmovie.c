/**
 * Author: biebera@msoe.edu <Andrew Bieber>
 * Class: CPE 2600 - Systems Programming
 * Lab 12: Multiprocessing 2 - mandelmovie
 *
 * Description:
 *  Generates a sequence of Mandelbrot frames that zoom in slightly on a
 *  specific region of the set. Uses multiple child processes running
 *  concurrently to speed up generation. The number of concurrent
 *  children is provided via the -p command line option.
 *
 * NOTE: ChatGPT(5.1) Used for skeletons and guidance. All 
 * code faciliated by me, Andrew Bieber
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static void show_help() {
    printf("Usage: ./mandelmovie -p num_processes -t num_threads\n");
}

int main(int argc, char *argv[]) {
    int num_processes = 1;
    int num_threads = 1;
    char c;

    // Parse command line
    while ((c = getopt(argc, argv, "p:t:h")) != -1) {
        switch (c) {
            case 'p':
                num_processes = atoi(optarg);
                break;
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'h':
                show_help();
                exit(0);
            default:
                show_help();
                exit(1);
        }
    }

    if (num_processes < 1) num_processes = 1;
    if (num_threads < 1) num_threads = 1;

    printf("mandelmovie: processes=%d threads=%d\n", num_processes, num_threads);

    // Convert thread count to string
    char t_str[16];
    snprintf(t_str, sizeof(t_str), "%d", num_threads);

    // Launch num_processes mandel processes
    for (int i = 0; i < num_processes; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // Child process
            char outfile[32];
            snprintf(outfile, sizeof(outfile), "mandel%d.jpg", i);

            execl("./mandel", "mandel",
                  "-t", t_str,        // ← NEW: pass thread count
                  "-o", outfile,      // output file
                  (char *)NULL);

            perror("execl failed");
            exit(1);
        }
    }

    // Parent waits for all children
    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    return 0;
}
