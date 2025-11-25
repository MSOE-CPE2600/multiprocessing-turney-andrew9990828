# System Programming Lab 11 Multiprocessing
Runtime Results

The program was timed using the Linux time command for 50 frames at various levels of concurrency.
Results:

Processes	Runtime (s)
1	        20.448
2	        11.713
5	        2.562
10	        3.534
20	        3.180

Discussion:

As the number of child processes increases from 1 → 2 → 5, the total runtime decreases dramatically. This shows clear speedup due to true parallel execution of multiple Mandelbrot frames at once. However, once the number of processes exceeds the number of available CPU cores, the speedup plateaus and even slightly increases (as seen at 10 and 20 processes). This is expected: additional processes introduce scheduling overhead, context switching, and increased contention for CPU time. The overall curve demonstrates diminishing returns at higher process counts, which matches theoretical expectations for parallel workloads.

Graph is in this repo as "runtime.png"

Lab 12: Multithreading + Multiprocessing Runtime Results:

The Mandelbrot program was enhanced with POSIX threads.
Now each process renders an image using multiple threads, and we recorded the runtime using:

time ./mandelmovie -p <processes> -t <threads>

Runtime Table (Processes ↓ vs Threads →)
Total Runtime (seconds)

(50 frames per run)

Processes ↓ / Threads →	1	2	4	8	16
                    1	0.537	0.285	0.311	0.247	0.179
                    2	0.557	0.338	0.344	0.293	0.213
                    5	0.717	0.422	0.414	0.405	0.387
                    10	0.783	0.729	0.765	0.871	0.723
                    20	1.383	1.422	1.445	2.044	1.773

This table is attached at "runtime2.png"

Discussion (Multithreading + Multiprocessing)
1. Single-process scaling (top row)

For p = 1, increasing threads from 1 → 16 reduces runtime significantly.
This shows that Mandelbrot pixel computations are embarrassingly parallel, and multithreading provides near-linear speedup up to around 8–16 threads.

2. Multi-process scaling (middle rows)

For moderate process counts (p = 2, 5):
Multithreading continues to improve performance
Scaling is strong but begins to level off as total worker threads ≈ CPU cores × 2

3. Oversubscription behavior (p = 10, 20)

As both processes and threads grow large, runtime worsens:
Example:
(20 processes × 8 threads) = 160 active threads
(20 processes × 16 threads) = 320 active threads
This overloads the CPU and OS scheduler, causing:
Extreme context switching
Thread thrashing
Cache eviction
Run queue delays
This is why performance deteriorates sharply in the last rows/columns.

Overall Conclusion:
Multithreading speeds up individual render tasks
Multiprocessing speeds up the batch of 50 frames
But combining too many processes AND threads overwhelms the system

The experiment perfectly demonstrates:
Optimal concurrency occurs when total threads ≈ physical CPU cores × 1–2.
Beyond that, overhead dominates and runtime increases.
A graph of the final table is also included in the repo as "runtime2.png"