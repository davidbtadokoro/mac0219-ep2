#! /bin/bash

set -o xtrace

MEASUREMENTS=15
INITIAL_NODES=1
NODES_ITERATIONS=5
INITIAL_NUM_THREADS=1
THREAD_ITERATIONS=5

NODES=$INITIAL_NODES
NUM_THREADS=$INITIAL_NUM_THREADS

NAMES=('mandelbrot_ompi_pth')
FILES=('triple_spiral.csv')

make
mkdir data

for NAME in ${NAMES[@]}; do
    mkdir data/$NAME

    for ((j=1; j<=$THREAD_ITERATIONS; j++)) do
        for ((i=1; i<=$NODES_ITERATIONS; i++)); do

                sudo perf stat -r $MEASUREMENTS mpirun --allow-run-as-root --oversubscribe -n $NODES $NAME -0.188 -0.012 0.554 0.754 4096 $NUM_THREADS

                # perf stat -r $MEASUREMENTS ./$NAME -2.5 1.5 -2.0 2.0 $NODES $NUM_THREADS >> /tmp/full.csv 2>&1
                # perf stat -r $MEASUREMENTS ./$NAME -0.8 -0.7 0.05 0.15 $NODES $NUM_THREADS>> /tmp/seahorse.csv 2>&1
                # perf stat -r $MEASUREMENTS ./$NAME 0.175 0.375 -0.1 0.1 $NODES $NUM_THREADS>> /tmp/elephant.csv 2>&1
                # perf stat -r $MEASUREMENTS ./$NAME -0.188 -0.012 0.554 0.754 $NODES $NUM_THREADS>> /tmp/triple_spiral.csv 2>&1

                for FILE in ${FILES[@]}; do
                    grep "time" /tmp/$FILE >> $FILE;    # Grava somente as linhas com o tempo decorrido no arquivo.
                    sed -i "s/         /$NUM_THREADS $NODES /g" $FILE; # Insere número de threads e número de NODES.

                    # Remove partes indesejadas da string
                    sed -i "s/+-/ /g" $FILE;
                    sed -i "s/(/ /g" $FILE;
                    sed -i "s/ )//g" $FILE;
                    sed -i "s/ seconds time elapsed / /g" $FILE;

                    # Substitui qualquer sequência de caracteres em branco por uma vírgula
                    sed -i "s/ \+/,/g" $FILE;
                    cat /dev/null > /tmp/$FILE
                done

                NODES=$(($NODES * 2))
        done

        NODES=$INITIAL_NODES
        NUM_THREADS=$(($NUM_THREADS * 2))

    done

    NUM_THREADS=$INITIAL_NUM_THREADS

    mv *.csv data/$NAME
    rm output.ppm
done
