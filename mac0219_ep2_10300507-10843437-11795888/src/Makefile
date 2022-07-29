OUTPUT=mandelbrot
CC=gcc
MPICC=mpicc
CFLAGS= -Wall -o

all: $(OUTPUT)_seq $(OUTPUT)_pth $(OUTPUT)_omp
all: $(OUTPUT)_ompi $(OUTPUT)_ompi_pth $(OUTPUT)_ompi_omp

$(OUTPUT)_seq: $(OUTPUT)_seq.c
	$(CC) $(OUTPUT)_seq.c -std=c99 -o $(OUTPUT)_seq

$(OUTPUT)_pth: $(OUTPUT)_pth.c
	$(CC) $(OUTPUT)_pth.c -std=c99 -pthread -o $(OUTPUT)_pth

$(OUTPUT)_omp: $(OUTPUT)_omp.c
	$(CC) $(OUTPUT)_omp.c -std=c99 -fopenmp -o $(OUTPUT)_omp

$(OUTPUT)_ompi: $(OUTPUT)_ompi.c
	$(MPICC) $(OUTPUT)_ompi.c -std=c99 -o $(OUTPUT)_ompi

$(OUTPUT)_ompi_pth: $(OUTPUT)_ompi_pth.c
	$(MPICC) $(OUTPUT)_ompi_pth.c -std=c99 -pthread -o $(OUTPUT)_ompi_pth

$(OUTPUT)_ompi_omp: $(OUTPUT)_ompi_omp.c
	$(MPICC) $(OUTPUT)_ompi_omp.c -std=c99 -fopenmp -o $(OUTPUT)_ompi_omp

.PHONY: clean
clean:
	rm $(OUTPUT)_seq $(OUTPUT)_pth $(OUTPUT)_omp \
	$(OUTPUT)_ompi $(OUTPUT)_ompi_pth $(OUTPUT)_ompi_omp \
	output.ppm
