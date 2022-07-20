#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <unistd.h>
#include <pthread.h>

/* Print an error message and exit with failure code */
#define DIE(...) { \
    fprintf(stderr, __VA_ARGS__); \
    exit(EXIT_FAILURE); \
};

/* A task containing the range of pixels that one thread takes care */
struct task {
    int start_pos;
    int end_pos;
};

/* Number of threads */
int n_threads;

/* Array that stores the threads IDs */
pthread_t * threads;

/* Array that stores the tasks for the threads*/
struct task *tasks;

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

void allocate_image_buffer(){
    int rgb_size = 3;
    image_buffer = (unsigned char **) malloc(sizeof(unsigned char *) * image_buffer_size);

    for(int i = 0; i < image_buffer_size; i++){
        image_buffer[i] = (unsigned char *) malloc(sizeof(unsigned char) * rgb_size);
    };
};

void init(int argc, char *argv[]){
    if(argc < 7){
        printf("usage: ./mandelbrot_omp c_x_min c_x_max c_y_min c_y_max image_size\n");
        printf("examples with image_size = 11500 and n_threads = 16:\n");
        printf("    Full Picture:         ./mandelbrot_pth -2.5 1.5 -2.0 2.0 11500 16\n");
        printf("    Seahorse Valley:      ./mandelbrot_pth -0.8 -0.7 0.05 0.15 11500 16\n");
        printf("    Elephant Valley:      ./mandelbrot_pth 0.175 0.375 -0.1 0.1 11500 16\n");
        printf("    Triple Spiral Valley: ./mandelbrot_pth -0.188 -0.012 0.554 0.754 11500 16\n");
        exit(0);
    }
    else{
        sscanf(argv[1], "%lf", &c_x_min);
        sscanf(argv[2], "%lf", &c_x_max);
        sscanf(argv[3], "%lf", &c_y_min);
        sscanf(argv[4], "%lf", &c_y_max);
        sscanf(argv[5], "%d", &image_size);
        sscanf(argv[6], "%d", &n_threads);

        i_x_max           = image_size;
        i_y_max           = image_size;
        image_buffer_size = image_size * image_size;

        pixel_width       = (c_x_max - c_x_min) / i_x_max;
        pixel_height      = (c_y_max - c_y_min) / i_y_max;
    };
    /* Initialization of array with threads IDs */
    if((threads = malloc(n_threads * sizeof(pthread_t))) == NULL)
        DIE("Threads malloc failed\n");
    /* Initialization of array with tasks of each thread */
    if((tasks = malloc(n_threads * sizeof(struct task))) == NULL)
        DIE("Tasks malloc failed\n");
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

void *thread_work(void *arg) {
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

    struct task * t = (struct task *) arg;

    for (int k = t->start_pos; k < t-> end_pos; k++) {
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
        /* update_rgb_buffer(iteration, col, row); */
    };

    return NULL;
};

void compute_mandelbrot(){
    int current_pos;
    int threads_with_one_more_work;
    int work_size;
    int k;

    /* Defining number of threads with one more pixel to care */
    threads_with_one_more_work = image_buffer_size % n_threads;

    /* Setting position of first pixel */
    current_pos = 0;

    /* For each thread: */
    for (k = 0; k < n_threads; ++k) {
        /* Getting the minimum num of pixels for this thread */
        work_size = image_buffer_size / n_threads;

        /* Is this one of the threads with one more pixel to care? */
        if (k < threads_with_one_more_work)
            work_size += 1;

        /* Defining the range of pixels that this thread takes care */
        tasks[k].start_pos = current_pos;
        tasks[k].end_pos = tasks[k].start_pos + work_size;
        current_pos = tasks[k].end_pos;

        /* Start this thread work */
        if(pthread_create(&threads[k], NULL, thread_work, (void *)&tasks[k]))
            DIE("Failed to create thread %d\n", k);
    };

    /* Joining the threads */
    for (k = 0; k < n_threads; ++k) {
        if(pthread_join(threads[k], NULL))
            DIE("failed to join thread %d\n", k);
    };

};

int main(int argc, char *argv[]){
    init(argc, argv);

    /* allocate_image_buffer(); */

    compute_mandelbrot();

    /* write_to_file(); */

    return 0;
};
