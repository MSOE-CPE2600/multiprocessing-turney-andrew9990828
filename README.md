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