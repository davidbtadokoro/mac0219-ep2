#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <pthread.h>

/* Opções de execução */
#define DEBUG 0
#define DO_OUTPUT 0

/* Parâmentros de execução */
#define MASTER 0
#define SIZE 4096

/* Parâmentros do PThreads */
#define N_THREADS 2

typedef struct {
    int start;
    int end;
} Pair;

double c_x_min;
double c_x_max;
double c_y_min;
double c_y_max;

double pixel_width;
double pixel_height;

int iteration_max = 200;

int image_size;
unsigned char **image_buffer;
unsigned char image[SIZE*SIZE];

int i_x_max;
int i_y_max;
int image_buffer_size;

int gradient_size = 16;
int colors[17][3] = {
                        {66, 30, 15},
                        {25, 7, 26},
                        {9, 1, 47},
                        {4, 4, 73},
                        {0, 7, 100},
                        {12, 44, 138},
                        {24, 82, 177},
                        {57, 125, 209},
                        {134, 181, 229},
                        {211, 236, 248},
                        {241, 233, 191},
                        {248, 201, 95},
                        {255, 170, 0},
                        {204, 128, 0},
                        {153, 87, 0},
                        {106, 52, 3},
                        {16, 16, 16},
                    };

void allocate_image_buffer(){
    int rgb_size = 3;
    image_buffer = (unsigned char **) malloc(sizeof(unsigned char *) * image_buffer_size);

    for(int i = 0; i < image_buffer_size; i++){
        image_buffer[i] = (unsigned char *) malloc(sizeof(unsigned char) * rgb_size);
    };
};

void init(int argc, char *argv[]){
    if(argc < 6){
        printf("usage: ./mandelbrot_seq c_x_min c_x_max c_y_min c_y_max image_size\n");
        printf("examples with image_size = 11500:\n");
        printf("    Full Picture:         ./mandelbrot_seq -2.5 1.5 -2.0 2.0 11500\n");
        printf("    Seahorse Valley:      ./mandelbrot_seq -0.8 -0.7 0.05 0.15 11500\n");
        printf("    Elephant Valley:      ./mandelbrot_seq 0.175 0.375 -0.1 0.1 11500\n");
        printf("    Triple Spiral Valley: ./mandelbrot_seq -0.188 -0.012 0.554 0.754 11500\n");
        exit(0);
    }
    else{
        sscanf(argv[1], "%lf", &c_x_min);
        sscanf(argv[2], "%lf", &c_x_max);
        sscanf(argv[3], "%lf", &c_y_min);
        sscanf(argv[4], "%lf", &c_y_max);
        sscanf(argv[5], "%d", &image_size);

        i_x_max           = image_size;
        i_y_max           = image_size;
        image_buffer_size = image_size * image_size;

        pixel_width       = (c_x_max - c_x_min) / i_x_max;
        pixel_height      = (c_y_max - c_y_min) / i_y_max;
    };
};

void update_rgb_buffer(int iteration, int x, int y){
    int color;

    if(iteration == iteration_max){
        image_buffer[(i_y_max * y) + x][0] = colors[gradient_size][0];
        image_buffer[(i_y_max * y) + x][1] = colors[gradient_size][1];
        image_buffer[(i_y_max * y) + x][2] = colors[gradient_size][2];
    }
    else{
        color = iteration % gradient_size;

        image_buffer[(i_y_max * y) + x][0] = colors[color][0];
        image_buffer[(i_y_max * y) + x][1] = colors[color][1];
        image_buffer[(i_y_max * y) + x][2] = colors[color][2];
    };
};

void write_to_file(){
    FILE * file;
    char * filename               = "output.ppm";
    char * comment                = "# ";

    int max_color_component_value = 255;

    file = fopen(filename,"wb");

    fprintf(file, "P6\n %s\n %d\n %d\n %d\n", comment,
            i_x_max, i_y_max, max_color_component_value);

    for(int i = 0; i < image_buffer_size; i++){
        fwrite(image_buffer[i], 1 , 3, file);
    };

    fclose(file);
};

void* compute_mandelbrot_aux(void* i_y_start_end_) {
    double z_x;
    double z_y;
    double z_x_squared;
    double z_y_squared;
    double escape_radius_squared = 4;

    int iteration;
    int i_x;
    int i_y;

    double c_x;
    double c_y;

    int i_y_start = ((Pair*) i_y_start_end_)->start;
    int i_y_end = ((Pair*) i_y_start_end_)->end;

    for(i_y = i_y_start; i_y < i_y_end; i_y++){
        c_y = c_y_min + i_y * pixel_height;

        if(fabs(c_y) < pixel_height / 2){
            c_y = 0.0;
        };

        for(i_x = 0; i_x < i_x_max; i_x++){
            c_x         = c_x_min + i_x * pixel_width;

            z_x         = 0.0;
            z_y         = 0.0;

            z_x_squared = 0.0;
            z_y_squared = 0.0;

            for(iteration = 0;
                iteration < iteration_max && \
                ((z_x_squared + z_y_squared) < escape_radius_squared);
                iteration++){
                z_y         = 2 * z_x * z_y + c_y;
                z_x         = z_x_squared - z_y_squared + c_x;

                z_x_squared = z_x * z_x;
                z_y_squared = z_y * z_y;
            };

            if(DO_OUTPUT) image[(i_y*SIZE) + i_x] = iteration;
        };
    };
}

void compute_mandelbrot(int start, int end) {
     /* inicialização relacionada a threads */
    pthread_t threads[N_THREADS];
    Pair* limits = calloc(N_THREADS, sizeof(Pair));
    int i_y, rc, t;

    /* loop principal, dividindo os i_y entre as threads */
    for (i_y = start, t=0; t < N_THREADS; t++) {
        limits[t].start = i_y;

        if (t == (N_THREADS - 1)) limits[t].end = end;
        else {
            i_y += (i_y_max / N_THREADS);
            limits[t].end = i_y;
        }

        rc = pthread_create(&threads[t], NULL, compute_mandelbrot_aux, (void*)(&limits[t]));
        
        if(rc) {
            fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        };
    };

    // junta threads
    for (int i = 0; i < N_THREADS; i++)
        pthread_join(threads[i], NULL);
}


int main(int argc, char *argv[]){
    init(argc, argv);

    int num_tasks, task_id, chunk_size, left_over, offset;
    int tag_offset = 1;
    int tag_image = 2;
    MPI_Status status;

    /* Inicializa OpenMPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

    chunk_size = (image_size / num_tasks);
    left_over = (image_size % num_tasks);
    offset = chunk_size + left_over; 

    if (task_id == MASTER) {
        if(DO_OUTPUT) allocate_image_buffer();

        for (int i = 1; i < num_tasks; i++) {
            printf("Process %d will receive offset: %d\n", i, offset);
            MPI_Send(&offset, 1, MPI_INT, i, tag_offset, MPI_COMM_WORLD);
            if(DO_OUTPUT) MPI_Send(&image[offset*SIZE], chunk_size*SIZE, MPI_UNSIGNED_CHAR,
              i, tag_image, MPI_COMM_WORLD);
            offset += chunk_size;
            
        }

        /* Calcula fatia da master */
        compute_mandelbrot(0, chunk_size+left_over);

        for (int i = 1; i < num_tasks; i++) {
            MPI_Recv(&offset, 1, MPI_INT, i, tag_offset, MPI_COMM_WORLD, &status);
            if(DO_OUTPUT) MPI_Recv(&image[offset*SIZE], chunk_size*SIZE, MPI_UNSIGNED_CHAR,
              i, tag_image, MPI_COMM_WORLD, &status);
        }

        if(DO_OUTPUT) {
            for(int i = 0; i < SIZE; i++) 
                for(int j = 0; j < SIZE; j++)
                    update_rgb_buffer(image[(j*SIZE)+i], i, j);

            write_to_file();
        }

    } else {
        MPI_Recv(&offset, 1, MPI_INT, MASTER, tag_offset, MPI_COMM_WORLD, &status);
        if(DO_OUTPUT) MPI_Recv(&image[offset*SIZE], chunk_size*SIZE, MPI_UNSIGNED_CHAR,
          MASTER, tag_image, MPI_COMM_WORLD, &status);
        if(DEBUG) printf("Process %d got offset: %d\n", task_id, offset);

        /* Calcula fatia do worker */
        compute_mandelbrot(offset, offset+chunk_size);

        MPI_Send(&offset, 1, MPI_INT, MASTER, tag_offset, MPI_COMM_WORLD);
        if(DO_OUTPUT) MPI_Send(&image[offset*SIZE], chunk_size*SIZE, MPI_UNSIGNED_CHAR,
          MASTER, tag_image, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
};
