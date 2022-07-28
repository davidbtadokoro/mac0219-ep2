#! /bin/bash

set -o xtrace

sudo perf stat -r 15 ./mandelbrot_seq -0.188 -0.012 0.554 0.754 4096 >> ./data/comparison/perf.output 2>&1

sudo perf stat -r 15 ./mandelbrot_pth -0.188 -0.012 0.554 0.754 4096 8 >> ./data/comparison/perf.output 2>&1

sudo perf stat -r 15 ./mandelbrot_omp -0.188 -0.012 0.554 0.754 4096 8 >> ./data/comparison/perf.output 2>&1

sudo perf stat -r 15 mpirun --allow-run-as-root --oversubscribe -n 8 mandelbrot_ompi -0.188 -0.012 0.554 0.754 4096 >> ./data/comparison/perf.output 2>&1

sudo perf stat -r 15 mpirun --allow-run-as-root --oversubscribe -n 2 mandelbrot_ompi_pth -0.188 -0.012 0.554 0.754 4096 4 >> ./data/comparison/perf.output 2>&1

sudo perf stat -r 15 mpirun --allow-run-as-root --oversubscribe -n 2 mandelbrot_ompi_omp -0.188 -0.012 0.554 0.754 4096 4 >> ./data/comparison/perf.output 2>&1

sudo rm output.ppm
