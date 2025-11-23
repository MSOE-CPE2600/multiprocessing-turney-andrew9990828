/// 
//  mandel.c
//  Based on example code found here:
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
// 
// Author: biebera@msoe.edu <Andrew Bieber>
// Class: CPE 2600 - Systems Programming
// Lab 12: Multiprocessing 2 - mandelmovie
//
//  Converted to use jpg instead of BMP and other minor changes
//  Extended for Lab 12 to support multithreading using pthreads.
//  
///

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>      // NEW: pthreads
#include "jpegrw.h"

// Maximum number of threads we will support (per lab spec)
#define MAX_THREADS 20

// ---------- local routines / declarations ----------

static int iteration_to_color(int i, int max);
static int iterations_at_point(double x, double y, int max);
static void compute_image(imgRawImage *img,
                          double xmin, double xmax,
                          double ymin, double ymax,
                          int max, int num_threads);
static void show_help(void);

// Struct used to pass work to each thread
typedef struct {
    imgRawImage *img;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    int max_iterations;
    int y_start;   // inclusive
    int y_end;     // exclusive
} thread_args_t;

// Thread function: compute a band of rows in the image
static void *thread_compute(void *arg) {
    thread_args_t *targs = (thread_args_t *)arg;

    imgRawImage *img = targs->img;
    int width  = img->width;
    int height = img->height;

    double xmin = targs->xmin;
    double xmax = targs->xmax;
    double ymin = targs->ymin;
    double ymax = targs->ymax;
    int max = targs->max_iterations;

    for (int j = targs->y_start; j < targs->y_end; ++j) {
        for (int i = 0; i < width; ++i) {
            // Determine the point in x,y space for that pixel.
            double x = xmin + i * (xmax - xmin) / width;
            double y = ymin + j * (ymax - ymin) / height;

            // Compute the iterations at that point.
            int iters = iterations_at_point(x, y, max);

            // Set the pixel in the bitmap.
            setPixelCOLOR(img, i, j, iteration_to_color(iters, max));
        }
    }

    return NULL;
}

// ---------- main program ----------

int main(int argc, char *argv[]) {
    char c;

    // These are the default configuration values used
    // if no command line arguments are given.
    const char *outfile = "mandel.jpg";
    double xcenter = 0;
    double ycenter = 0;
    double xscale  = 4;
    double yscale  = 0; // calc later
    int    image_width  = 1000;
    int    image_height = 1000;
    int    max = 1000;
    int    num_threads = 1;   // NEW: default to single-threaded

    // For each command line argument given,
    // override the appropriate configuration value.

    // NEW: added 't:' option
    while ((c = getopt(argc, argv, "x:y:s:W:H:m:o:t:h")) != -1) {
        switch (c) {
            case 'x':
                xcenter = atof(optarg);
                break;
            case 'y':
                ycenter = atof(optarg);
                break;
            case 's':
                xscale = atof(optarg);
                break;
            case 'W':
                image_width = atoi(optarg);
                break;
            case 'H':
                image_height = atoi(optarg);
                break;
            case 'm':
                max = atoi(optarg);
                break;
            case 'o':
                outfile = optarg;
                break;
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'h':
                show_help();
                exit(0);
                break;
            default:
                show_help();
                exit(1);
        }
    }

    // Clamp thread count to [1, MAX_THREADS]
    if (num_threads < 1) {
        num_threads = 1;
    } else if (num_threads > MAX_THREADS) {
        num_threads = MAX_THREADS;
    }

    // Calculate y scale based on x scale (settable) and image sizes in X and Y (settable)
    yscale = xscale / image_width * image_height;

    // Display the configuration of the image.
    printf("mandel: x=%lf y=%lf xscale=%lf yscale=%1f max=%d threads=%d outfile=%s\n",
           xcenter, ycenter, xscale, yscale, max, num_threads, outfile);

    // Create a raw image of the appropriate size.
    imgRawImage *img = initRawImage(image_width, image_height);

    // Fill it with black
    setImageCOLOR(img, 0);

    // Compute the Mandelbrot image (now possibly multithreaded)
    compute_image(img,
                  xcenter - xscale / 2,
                  xcenter + xscale / 2,
                  ycenter - yscale / 2,
                  ycenter + yscale / 2,
                  max,
                  num_threads);

    // Save the image in the stated file.
    storeJpegImageFile(img, outfile);

    // free the mallocs
    freeRawImage(img);

    return 0;
}


// ---------- computation helpers ----------

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/
int iterations_at_point(double x, double y, int max) {
    double x0 = x;
    double y0 = y;

    int iter = 0;

    while ((x * x + y * y <= 4) && iter < max) {
        double xt = x * x - y * y + x0;
        double yt = 2 * x * y + y0;

        x = xt;
        y = yt;

        iter++;
    }

    return iter;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max".
Now implemented using num_threads pthreads.
*/
void compute_image(imgRawImage *img,
                   double xmin, double xmax,
                   double ymin, double ymax,
                   int max, int num_threads) {
    int height = img->height;

    // If only one thread was requested, we can run directly in this thread
    if (num_threads == 1) {
        thread_args_t args;
        args.img = img;
        args.xmin = xmin;
        args.xmax = xmax;
        args.ymin = ymin;
        args.ymax = ymax;
        args.max_iterations = max;
        args.y_start = 0;
        args.y_end = height;

        thread_compute(&args);
        return;
    }

    // For multiple threads, split the rows into num_threads bands
    if (num_threads > MAX_THREADS) {
        num_threads = MAX_THREADS;
    }

    pthread_t threads[MAX_THREADS];
    thread_args_t targs[MAX_THREADS];

    for (int t = 0; t < num_threads; ++t) {
        int y_start = (t * height) / num_threads;
        int y_end   = ((t + 1) * height) / num_threads;

        targs[t].img = img;
        targs[t].xmin = xmin;
        targs[t].xmax = xmax;
        targs[t].ymin = ymin;
        targs[t].ymax = ymax;
        targs[t].max_iterations = max;
        targs[t].y_start = y_start;
        targs[t].y_end = y_end;

        int rc = pthread_create(&threads[t], NULL, thread_compute, &targs[t]);
        if (rc != 0) {
            // If thread creation fails, fall back to doing this band in the main thread
            fprintf(stderr, "pthread_create failed for thread %d, running synchronously\n", t);
            thread_compute(&targs[t]);
            // Mark this thread as "not started" so we don't join it later
            threads[t] = 0;
        }
    }

    // Join all threads that were successfully started
    for (int t = 0; t < num_threads; ++t) {
        if (threads[t] != 0) {
            pthread_join(threads[t], NULL);
        }
    }
}

/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of max.
Modify this function to make more interesting colors.
*/
int iteration_to_color(int iters, int max) {
    int color = 0xFFFFFF * iters / (double)max;
    return color;
}


// ---------- help text ----------

void show_help(void) {
    printf("Use: mandel [options]\n");
    printf("Where options are:\n");
    printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
    printf("-x <coord>  X coordinate of image center point. (default=0)\n");
    printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
    printf("-s <scale>  Scale of the image in Mandelbrot coordinates (X-axis). (default=4)\n");
    printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
    printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
    printf("-o <file>   Set output file. (default=mandel.jpg)\n");
    printf("-t <num>    Number of threads to use (1-%d, default=1)\n", MAX_THREADS);
    printf("-h          Show this help text.\n");
    printf("\nSome examples are:\n");
    printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
    printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
    printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000 -t 8\n\n");
}
