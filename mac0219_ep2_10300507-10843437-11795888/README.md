# MAC0219 - Exercício-Programa 2

Integrantes:
* David de Barros Tadokoro         - 10300507  
* Fernando Henrique Junqueira      - 11795888
* Guilherme Moreno Silva           - 10843437  


## Compilação

Para compilar os código basta acessar o diretório `src/` e rodar
```
$ make
```
Irão ser compilados os programas `mandelbrot_seq`, `mandelbrot_pth`, `mandelbrot_omp`, `mandelbrot_ompi`, `mandelbrot_ompi_pth`, `mandelbrot_ompi_omp`.

## Execução

Tendo gerado os executáveis no diretório `src/`, as implementações podem ser executadas como descrito a seguir.

Para executar os programas `mandelbrot_seq` e `mandelbrot_ompi`, siga o padrão
```
$ ./mandelbrot_<seq/ompi> c_x_min c_x_max c_y_min c_y_max image_size
```

As versões paralelizadas, seja via Pthreads ou OpenMP, independentemente de fazerem uso ou não de OpenMPI, devem conter um argumento adicional, referente ao número de threads desejados na execução. Para executá-las, siga o padrão
```
$ ./mandelbrot_<pth/omp/ompi_pth/ompi_omp> c_x_min c_x_max c_y_min c_y_max image_size n_threads
```
Note `n_threads` como último argumento.

Nos casos que utilizam OpenMPI, estas execuções devem ser precedidas do utilitário `mpirun` ou `mpiexec` da seguinte forma
```
$ <mpirun/mpiexec> -np <n_procs> <program>
```
Acima, `n_procs` se refere ao número de nós (processo) em que se deseja executar o programa `program` (este sendo da forma dos anteriores).
