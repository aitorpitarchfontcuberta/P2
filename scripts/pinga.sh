#! /bin/bash

for alpha0 in $(seq 0.5 0.01 1.5); do
    echo -ne "alpha0 = $alpha0:\t"
    scripts/run_vad.sh $alpha0 | grep TOTAL
done | sort -t: -k 3n