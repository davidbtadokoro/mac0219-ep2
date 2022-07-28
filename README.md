# MAC0219 - Exercício-Programa 2

Integrantes:
* David de Barros Tadokoro         - 10300507  
* Fernando Henrique Junqueira      - 11795888 
* Guilherme Moreno Silva           - 10843437  


## Compilação
Para compilar os código basta acessar o diretório e rodar
```
$ make
```
Irão ser compilados os programas mandelbrot_seq, mandelbrot_pth
mandelbrot_omp, mandelbrot_ompi, mandelbrot_ompi_pth, mandelbrot_ompi_omp.

## Execução
Para executar o programa mandelbrot_seq basta executá-lo
normalmente. As versões paralelizadas, seja via Pthreads ou OpenMP,
independentemente de fazerem uso ou não OpenMPI, devem contém um
argumento adicional, referente ao número de threads desejados na
paralelização. Assim, para executar, por exemplo, a região Triple
Spiral Valley na versão com OpenMPI e oito threads via Pthreads,
o comando deve ser
```
$ ./mandelbrot_ompi_pth -0.188 -0.012 0.554 0.754 4096 8
```
Note o oito como último argumento.

É importante lembrar também que as versões com suporte ao OpenMPI
devem ser executadas com o utilitário `mpiexec`.
