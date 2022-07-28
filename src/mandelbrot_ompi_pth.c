#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <string.h>
#include <pthread.h>

double c_x_min;
double c_x_max;
double c_y_min;
double c_y_max;

double pixel_width;
double pixel_height;

int iteration_max = 200;

int image_size;
unsigned char **image_buffer;

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

/* OpenMPI global variables */
int num_procs;                // total number of nodes
int rank;                     // rank of node
int procs_with_one_more_pos;  // nodes with one more position in task
int task_size;                // size of task
int beg_index;                // beggining index of task
int end_index;                // ending index of task

/* PThreads structs and global variables */
int num_threads;

typedef struct {
    int start;
    int end;
} Pair;

/* ------------------------------ Initialization ------------------------------ */

void init(int argc, char *argv[]){
    if(argc < 7){
        printf("usage: ./mandelbrot_seq c_x_min c_x_max c_y_min c_y_max image_size num_threads\n");
        printf("examples with image_size = 11500:\n");
        printf("    Full Picture:         ./mandelbrot_ompi_pth -2.5 1.5 -2.0 2.0 11500 8\n");
        printf("    Seahorse Valley:      ./mandelbrot_ompi_pth -0.8 -0.7 0.05 0.15 11500 8\n");
        printf("    Elephant Valley:      ./mandelbrot_ompi_pth 0.175 0.375 -0.1 0.1 11500 8\n");
        printf("    Triple Spiral Valley: ./mandelbrot_ompi_pth -0.188 -0.012 0.554 0.754 11500 8\n");
        exit(0);
    }
    else{
        sscanf(argv[1], "%lf", &c_x_min);
        sscanf(argv[2], "%lf", &c_x_max);
        sscanf(argv[3], "%lf", &c_y_min);
        sscanf(argv[4], "%lf", &c_y_max);
        sscanf(argv[5], "%d", &image_size);
        sscanf(argv[6], "%d", &num_threads);

        i_x_max           = image_size;
        i_y_max           = image_size;
        image_buffer_size = image_size * image_size;

        pixel_width       = (c_x_max - c_x_min) / i_x_max;
        pixel_height      = (c_y_max - c_y_min) / i_y_max;
    };
};

/* ------------------------------ Initialization ------------------------------ */

/* ------------------------------ I/O & Allocation ------------------------------ */

void allocate_image_buffer(){
    int rgb_size = 3;

    image_buffer = (unsigned char **) malloc(sizeof(unsigned char *) * image_buffer_size);

    for(int i = 0; i < image_buffer_size; i++){
        image_buffer[i] = (unsigned char *) malloc(sizeof(unsigned char) * rgb_size);
    };
};

void deallocate_image_buffer(){
    int rgb_size = 3;

    for (int i = 0; i < image_buffer_size; i++){ free(image_buffer[i]); };

    free(image_buffer);
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

/* ------------------------------ I/O & Allocation ------------------------------ */

/* ------------------------------ Computation ------------------------------ */

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

void compute_mandelbrot_old(int beg_index, int end_index){
    double z_x;
    double z_y;
    double z_x_squared;
    double z_y_squared;
    double escape_radius_squared = 4;

    int iteration;
    int row;
    int col;

    double c_x;
    double c_y;

    for (int k = beg_index; k < end_index; k++) {
        /* Converting array indexes to matrix indexes */
        col = k % i_y_max;
        row = k / i_y_max;

        /* Defining complex number coordinates */
        c_x = c_x_min + col * pixel_width;
        c_y = c_y_min + row * pixel_height;

        if(fabs(c_y) < pixel_height / 2) {
            c_y = 0.0;
        };

        /* Resetting z and z^2 */
        z_x = 0.0;
        z_y = 0.0;
        z_x_squared = 0.0;
        z_y_squared = 0.0;

        /* Define pixel color */
        for(iteration = 0;
            iteration < iteration_max && \
            ((z_x_squared + z_y_squared) < escape_radius_squared);
            iteration++){
            z_y         = 2 * z_x * z_y + c_y;
            z_x         = z_x_squared - z_y_squared + c_x;

            z_x_squared = z_x * z_x;
            z_y_squared = z_y * z_y;
        };

        /* Commit pixel color */
        //update_rgb_buffer(iteration, k);
    };
};

void* compute_mandelbrot_aux(void* arg) {
    double z_x;
    double z_y;
    double z_x_squared;
    double z_y_squared;
    double escape_radius_squared = 4;

    int iteration;
    int col;
    int row;

    double c_x;
    double c_y;

    Pair * t = (Pair *) arg;

    for (int k = t->start; k < t->end; k++) {
        /* Converting array indexes to matrix indexes */
        col = k % i_y_max;
        row = k / i_y_max;

        /* Defining complex number coordinates */
        c_x = c_x_min + col * pixel_width;
        c_y = c_y_min + row * pixel_height;

        if(fabs(c_y) < pixel_height / 2) {
            c_y = 0.0;
        };

        /* Resetting z and z^2 */
        z_x = 0.0;
        z_y = 0.0;
        z_x_squared = 0.0;
        z_y_squared = 0.0;

        /* Define pixel color */
        for(iteration = 0;
            iteration < iteration_max && \
            ((z_x_squared + z_y_squared) < escape_radius_squared);
            iteration++){
            z_y         = 2 * z_x * z_y + c_y;
            z_x         = z_x_squared - z_y_squared + c_x;

            z_x_squared = z_x * z_x;
            z_y_squared = z_y * z_y;
        };

        /* Commit pixel color */
        update_rgb_buffer(iteration, col, row);
    };

    return NULL;
};

void compute_mandelbrot(int start, int end) {
     /* initialization related to threads */
    pthread_t threads[num_threads];
    Pair* limits = calloc(num_threads, sizeof(Pair));
    int index, rc, t;

    /* sharing the indexes between the threads */
    for (index = start, t=0; t < num_threads; t++) {
        limits[t].start = index;

        if (t == (num_threads - 1)) limits[t].end = end;
        else {
            index += ((end - start) / num_threads);
            limits[t].end = index;
        }

        rc = pthread_create(&threads[t], NULL, compute_mandelbrot_aux, (void*)(&limits[t]));

        if(rc) {
            fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        };
    };

    // join threads
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);
}

/* ------------------------------ Computation ------------------------------ */

/* ------------------------------ OpenMPI ------------------------------ */

/*
 *  This function initializes OpenMPI and defines the task interval for
 *  each worker. It also defines who is the master and who are the workers.
 *  By calling it, the program starts distributed execution.
 */
void init_mpi(){
    MPI_Init(NULL, NULL);

    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Defining number of nodes with one more position as responsability
    procs_with_one_more_pos = (image_buffer_size % num_procs);

    if (rank < procs_with_one_more_pos) {
        task_size = (image_buffer_size / num_procs) + 1;
        beg_index = rank * task_size;
        end_index = beg_index + task_size;
    }
    else {
        task_size = (image_buffer_size / num_procs);
        beg_index = procs_with_one_more_pos * (task_size+1);
        beg_index += (rank-procs_with_one_more_pos) * task_size;
        end_index = beg_index + task_size;
    };

};

/*
 *  This function terminates the separate worker nodes and merges the results
 *  in the master node. After execution, the program reverts to a sequential flow.
 */
void term_mpi(){
    int i, j;
    int worker_rank;
    MPI_Status status;
    unsigned char * task_buffer = malloc(sizeof(unsigned char) * (task_size*3));

    // If it is a worker
    if (rank) {

        for (i = 0; i < task_size; i++) {
          task_buffer[i*3] = image_buffer[beg_index + i][0];
          task_buffer[i*3 + 1] = image_buffer[beg_index + i][1];
          task_buffer[i*3 + 2] = image_buffer[beg_index + i][2];
        }

        MPI_Send(task_buffer, task_size*3, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
    }
    // If it is the master
    else {
        for (i = 1; i < num_procs; i++) {
            MPI_Recv(task_buffer, task_size*3+3, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE,
                     MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            worker_rank = status.MPI_SOURCE;

            if (worker_rank < procs_with_one_more_pos) {
                task_size = (image_buffer_size / num_procs) + 1;
                beg_index = worker_rank * task_size;
            }
            else {
                task_size = (image_buffer_size / num_procs);
                beg_index = procs_with_one_more_pos * (task_size+1);
                beg_index += (worker_rank-procs_with_one_more_pos) * task_size;
            };

            for (j = 0; j < task_size; j++) {
                image_buffer[beg_index + j][0] = (unsigned char) task_buffer[j*3];
                image_buffer[beg_index + j][1] = (unsigned char) task_buffer[j*3 + 1];
                image_buffer[beg_index + j][2] = (unsigned char) task_buffer[j*3 + 2];
            };
        };

        write_to_file();
    };

    free(task_buffer);

    MPI_Finalize();
};

/* ------------------------------ OpenMPI ------------------------------ */

/* ------------------------------ Main ------------------------------ */

int main(int argc, char *argv[]){
    init(argc, argv);

    allocate_image_buffer();

    /* Distributed block */
    init_mpi();
    compute_mandelbrot(beg_index, end_index);
    term_mpi();

    deallocate_image_buffer();

    return 0;
};

/* ------------------------------ Main ------------------------------ */
