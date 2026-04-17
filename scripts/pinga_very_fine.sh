#!/bin/bash

# Búsqueda muy fina de alpha0 con pasos de 0.0005 entre 1.034 y 1.042

for alpha0 in $(seq 1.034 0.0005 1.042); do
    echo -ne "alpha0 = $alpha0:\t"
    scripts/run_vad.sh $alpha0 | grep TOTAL
done | sort -t: -k 3nr | head -20
