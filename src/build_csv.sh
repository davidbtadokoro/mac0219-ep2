#! /bin/bash

set -o xtrace

MEASUREMENTS=10
INITIAL_SIZE=16
SIZE_ITERATIONS=6
INITIAL_NUM_THREADS=1
THREAD_ITERATIONS=6

SIZE=$INITIAL_SIZE
NUM_THREADS=$INITIAL_NUM_THREADS

NAMES=('mandelbrot_seq' 'mandelbrot_pth' 'mandelbrot_omp')
FILES=('full.csv' 'seahorse.csv' 'elephant.csv' 'triple_spiral.csv')

make
mkdir data

for NAME in ${NAMES[@]}; do
    mkdir data/$NAME

    for ((j=1; j<=$THREAD_ITERATIONS; j++)) do
        for ((i=1; i<=$SIZE_ITERATIONS; i++)); do
                perf stat -r $MEASUREMENTS ./$NAME -2.5 1.5 -2.0 2.0 $SIZE $NUM_THREADS >> /tmp/full.csv 2>&1
                perf stat -r $MEASUREMENTS ./$NAME -0.8 -0.7 0.05 0.15 $SIZE $NUM_THREADS>> /tmp/seahorse.csv 2>&1
                perf stat -r $MEASUREMENTS ./$NAME 0.175 0.375 -0.1 0.1 $SIZE $NUM_THREADS>> /tmp/elephant.csv 2>&1
                perf stat -r $MEASUREMENTS ./$NAME -0.188 -0.012 0.554 0.754 $SIZE $NUM_THREADS>> /tmp/triple_spiral.csv 2>&1

                for FILE in ${FILES[@]}; do
                    grep "time" /tmp/$FILE >> $FILE;    # Grava somente as linhas com o tempo decorrido no arquivo.
                    sed -i "s/         /$NUM_THREADS $SIZE /g" $FILE; # Insere número de threads e tamanho de entrada.
                    
                    # Remove partes indesejadas da string
                    sed -i "s/+-/ /g" $FILE;
                    sed -i "s/(/ /g" $FILE;
                    sed -i "s/ )//g" $FILE;
                    sed -i "s/ seconds time elapsed / /g" $FILE;

                    # Substitui qualquer sequência de caracteres em branco por uma vírgula
                    sed -i "s/ \+/,/g" $FILE;
                    cat /dev/null > /tmp/$FILE
                done

                SIZE=$(($SIZE * 2))
        done

        SIZE=$INITIAL_SIZE
        NUM_THREADS=$(($NUM_THREADS * 2))
        
        if [ $NAME == "mandelbrot_seq" ]; then
            break;
        fi
    done

    NUM_THREADS=$INITIAL_NUM_THREADS

    mv *.csv data/$NAME
    rm output.ppm
done
