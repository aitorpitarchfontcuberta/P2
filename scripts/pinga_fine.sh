#!/bin/bash

# Búsqueda fina de alpha0 con pasos de 0.002 entre 0.99 y 1.10

for alpha0 in $(seq 0.99 0.002 1.10); do
    echo -ne "alpha0 = $alpha0:\t"
    scripts/run_vad.sh $alpha0 | grep TOTAL
done | sort -t: -k 3nr | head -20
