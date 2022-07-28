CC=gcc
MPICC=mpicc
CFLAGS= -Wall -o

all: mandelbrot_seq mandelbrot_pth mandelbrot_omp 
all: mandelbrot_ompi mandelbrot_ompi_pth mandelbrot_ompi_omp

mandelbrot_seq: mandelbrot_seq.c
	$(CC) mandelbrot_seq.c -std=c99 -pthread -o mandelbrot_seq

mandelbrot_pth: mandelbrot_pth.c
	$(CC) mandelbrot_pth.c -std=c99 -pthread -o mandelbrot_pth

mandelbrot_omp: mandelbrot_omp.c
	$(CC) mandelbrot_omp.c -std=c99 -fopenmp -o mandelbrot_omp

mandelbrot_ompi: mandelbrot_ompi.c
	$(MPICC) mandelbrot_ompi.c -std=c99 -pthread -o mandelbrot_ompi

mandelbrot_ompi_pth: mandelbrot_ompi_pth.c
	$(MPICC) mandelbrot_ompi_pth.c -std=c99 -pthread -o mandelbrot_ompi_pth

mandelbrot_ompi_omp: mandelbrot_ompi_omp.c
	$(MPICC) mandelbrot_ompi_omp.c -std=c99 -fopenmp -o mandelbrot_ompi_omp

clean:
	rm mandelbrot_seq mandelbrot_pth mandelbrot_omp \
	mandelbrot_ompi mandelbrot_ompi_pth mandelbrot_ompi_omp
