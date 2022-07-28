1;

data_ompi = csvread('./data/mandelbrot_ompi/triple_spiral.csv');
data_ompi_pth = csvread('./data/mandelbrot_ompi_pth/triple_spiral.csv');
data_ompi_omp = csvread('./data/mandelbrot_ompi_omp/triple_spiral.csv');

global SIZE = 5;
global THREADS = 5;
global colors=['r', 'b', 'y', 'g', 'c', 'm'];

function plot_ompi(data)
  line(data(1:end, 1),
       data(1:end, 2),
       "linewidth", 1,
       "color", 'r',
       "marker", ".",
       "markersize", 8,
       "clipping", "on");

  xlabel("Number of Nodes");
  ylabel("Time Elapsed (seconds)");
  box on;
  grid on;

endfunction

function plot_ompi_pth(data)
  global SIZE;
  global THREADS;
  global colors;

  row = 1;
  for i = 1:THREADS
    line(data(row:row + SIZE - 1, 2),
      data(row:row + SIZE - 1, 3),
      "linewidth", 1,
      "color", colors(i+1),
      "marker", ".",
      "markersize", 8,
      "clipping", "on");

    row += SIZE;
  end

  xlabel("Number of Nodes");
  ylabel("Time Elapsed (seconds)");
  legend("1 thread", "2 threads", "4 threads", "8 threads", "16 threads", "location", "northwest");
  box on;
  grid on;

endfunction

function plot_ompi_omp(data)
  global SIZE;
  global THREADS;
  global colors;

  row = 1;
  for i = 1:THREADS
    line(data(row:row + SIZE - 1, 2),
      data(row:row + SIZE - 1, 3),
      "linewidth", 1,
      "color", colors(i+1),
      "marker", ".",
      "markersize", 8,
      "clipping", "on");

    row += SIZE;
  end

  xlabel("Number of Nodes");
  ylabel("Time Elapsed (seconds)");
  legend("1 thread", "2 threads", "4 threads", "8 threads", "16 threads", "location", "northeast");
  box on;
  grid on;

endfunction

# CLEARING
close all

# OMPI
figure(1);
title("OpenMPI", "FontSize", 22, 'FontName', 'SansSerif');
plot_ompi(data_ompi);
saveas(1, "ompi", "jpg");
close;

# OMPI+PTH
figure(2)
title("OpenMPI + Pthreads", "FontSize", 22, 'FontName', 'SansSerif');
plot_ompi_pth(data_ompi_pth);
saveas(2, "ompi_pth", "jpg");
close;

# OMPI+OMP
figure(3)
title("OpenMPI + OpenMP", "FontSize", 22, 'FontName', 'SansSerif');
plot_ompi_omp(data_ompi_omp);
saveas(3, "ompi_omp", "jpg");
close;
