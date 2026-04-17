#!/bin/bash

# Prueba variaciones de N_INIT manteniendo alpha0=1.0375

ALPHA0=1.0375
RESULTS_FILE="/tmp/ninit_results.txt"
> $RESULTS_FILE

for N_INIT_VAL in 12 13 14 15 16 17 18; do
    echo "Probando N_INIT = $N_INIT_VAL..."
    
    # Editar vad.h temporalmente
    sed -i.bak "s/#define N_INIT.*/#define N_INIT       $N_INIT_VAL/" src/vad.h
    
    # Compilar
    meson compile -C bin > /dev/null 2>&1
    
    # Ejecutar con alpha0 óptimo
    RESULT=$(scripts/run_vad.sh $ALPHA0 2>/dev/null | grep "TOTAL" | tail -1)
    echo "N_INIT=$N_INIT_VAL: $RESULT" >> $RESULTS_FILE
    echo "N_INIT=$N_INIT_VAL: $RESULT"
done

# Restaurar original
sed -i.bak "s/#define N_INIT.*/#define N_INIT       15/" src/vad.h
meson compile -C bin > /dev/null 2>&1

echo ""
echo "=== RESUMEN ==="
cat $RESULTS_FILE | sort -t: -k 3nr
