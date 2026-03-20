#include <math.h>
#include "pav_analysis.h"

float compute_power(const float *x, unsigned int N) {
    float sum_sq = 0;
    for (unsigned int n = 0; n < N; n++) {
        sum_sq += x[n] * x[n]; // Suma de los cuadrados de las muestras [1]
    }

    float mean_sq = sum_sq / N;
    // Se utiliza un valor mínimo para evitar logaritmo de cero si la señal es silencio absoluto
    if (mean_sq <= 0) return -100.0f; 

    return 10.0 * log10(mean_sq); // El resultado se devuelve en decibelios (dB) [1]
}

float compute_am(const float *x, unsigned int N) {
    float sum_abs = 0;
    for (unsigned int n = 0; n < N; n++) {
        sum_abs += fabsf(x[n]); // Suma de los valores absolutos [1]
    }
    return sum_abs / N; 
}

float compute_zcr(const float *x, unsigned int N, float fm) {
    unsigned int count = 0;
    for (unsigned int n = 1; n < N; n++) {
        // Se detecta un cambio de signo entre la muestra actual y la anterior [1]
        // Un método común es comprobar si el producto es negativo, 
        // aunque el enunciado sugiere comparar las funciones signo.
        if ((x[n] > 0 && x[n-1] < 0) || (x[n] < 0 && x[n-1] > 0)) {
            count++;
        }
    }
    // Normalización con respecto al inverso de la frecuencia de Nyquist [1]
    return (fm / 2.0) * (count / (float)(N - 1));

}

float compute_power_windowed(const float *x, const float *w, unsigned int N) {
    float num = 0.0, den = 0.0;
    for (unsigned int n = 0; n < N; n++) {
        num += (x[n] * w[n]) * (x[n] * w[n]);
        den += w[n] * w[n];
    }
    if (den <= 0 || num <= 0) return -100.0f;
    return 10.0f * log10f(num / den);
}