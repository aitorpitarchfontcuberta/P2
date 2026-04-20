#! /bin/bash


for alpha0 in $(seq 1.034 0.0005 1.045); do
    echo -ne "alpha0 = $alpha0:\t"
    scripts/run_vad.sh $alpha0 | grep TOTAL
done | sort -t: -k 3nr | head -20
