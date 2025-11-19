/**
 * Author: biebera@msoe.edu <Andrew Bieber>
 * Class: CPE 2600 - Systems Programming
 * Lab 11: Multiprocessing - mandelmovie
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
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <math.h>

#define DEFAULT_FRAMES 50

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s -p num_processes\n", prog);
}

int main(int argc, char *argv[]) {
    int opt;
    int max_procs = 0;               // Required argument
    int frames = DEFAULT_FRAMES;     // Always 50 unless changed

   
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
            max_procs = atoi(optarg);
            break;
        default:
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (max_procs <= 0) {
        usage(argv[0]);
        fprintf(stderr, "Error: must provide -p with a value > 0.\n");
        return EXIT_FAILURE;
    }

    // ---------------------------
    // Zoom-in parameters
    // ---------------------------
    const double center_x = -0.743643887037151;   
    const double center_y =  0.131825904205330;

    const double start_scale = 0.004;             // initial zoom level
    const double zoom_factor = 0.97;              // shrink 3% per frame

    const int width  = 800;
    const int height = 600;

    int active = 0;

    // ---------------------------
    // Generate frames
    // ---------------------------
    for (int frame = 0; frame < frames; frame++) {

        // If we hit max children, wait for one to finish
        while (active >= max_procs) {
            pid_t w = wait(NULL);
            if (w > 0)
                active--;
            else if (w == -1 && errno == ECHILD)
                break;
        }

        // Compute this frame's scale value
        double scale = start_scale * pow(zoom_factor, frame);

        // Convert numeric values to strings
        char x_str[64], y_str[64], s_str[64], w_str[16], h_str[16], outfile[64];
        snprintf(x_str, sizeof(x_str), "%.15f", center_x);
        snprintf(y_str, sizeof(y_str), "%.15f", center_y);
        snprintf(s_str, sizeof(s_str), "%.15f", scale);
        snprintf(w_str, sizeof(w_str), "%d", width);
        snprintf(h_str, sizeof(h_str), "%d", height);
        snprintf(outfile, sizeof(outfile), "mandel%d.jpg", frame);

        // ---------------------------
        // Fork child to exec mandel
        // ---------------------------
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            // Try waiting and retrying frame
            pid_t w = wait(NULL);
            if (w > 0) active--;
            frame--;
            continue;
        }

        if (pid == 0) {
            // Child → replace with mandel
            execl("./mandel",
                  "mandel",
                  "-x", x_str,
                  "-y", y_str,
                  "-s", s_str,
                  "-W", w_str,
                  "-H", h_str,
                  "-o", outfile,
                  (char *)NULL);

            // If execl failed:
            perror("execl");
            _exit(EXIT_FAILURE);
        }

        // Parent continues
        active++;
    }

    // ---------------------------
    // Wait for remaining children
    // ---------------------------
    while (active > 0) {
        pid_t w = wait(NULL);
        if (w > 0)
            active--;
        else if (w == -1 && errno == ECHILD)
            break;
    }

    return EXIT_SUCCESS;
}
