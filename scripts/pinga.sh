#! /bin/bash

for alpha0 in $(seq 1 0.01 1.1); do
    echo -ne "alpha0 = $alpha0:\t"
    scripts/run_vad.sh $alpha0 | grep TOTAL
done | sort -t: -k 3n