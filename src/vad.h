#ifndef _VAD_H
#define _VAD_H
#include <stdio.h>

typedef enum {
  ST_UNDEF=0, ST_SILENCE, ST_VOICE, ST_INIT,
  ST_MAYBE_SILENCE, ST_MAYBE_VOICE
} VAD_STATE;

const char *state2str(VAD_STATE st);

#define N_INIT       15   /* tramas iniciales para calcular k0 */
#define ALPHA1       2.5F /* dB sobre k0 para entrar en maybe_voice */
#define ALPHA2       2.0F /* dB adicionales para confirmar voz */
#define MIN_VOICE_F  2     /* tramas mínimas para confirmar voz */
#define MIN_SIL_F    25   /* tramas mínimas para confirmar silencio */

typedef struct {
  VAD_STATE state;
  float     sampling_rate;
  unsigned int frame_length;
  float     last_feature;

  /* parámetros de umbral */
  float     alpha0;  /* parámetro de optimización */
  float     alpha1;  /* dB sobre k0 para entrar en maybe_voice */
  float     alpha2;  /* dB adicionales para confirmar voz */

  /* umbrales */
  float     k0;   /* nivel de ruido de fondo */
  float     k1;   /* k0 + alpha1: umbral maybe_voice */
  float     k2;   /* k1 + alpha2: umbral confirmación voz */

  /* acumulador para el cálculo de k0 */
  float     sum_power;
  int       init_count;

  /* contadores de duración */
  int       frame_count; /* tramas en estado maybe actual */
} VAD_DATA;

VAD_DATA    *vad_open(float sampling_rate, float alpha0, float alpha1, float alpha2);
unsigned int vad_frame_size(VAD_DATA *);
VAD_STATE    vad(VAD_DATA *vad_data, float *x);
VAD_STATE    vad_close(VAD_DATA *vad_data);
void         vad_show_state(const VAD_DATA *, FILE *);

#endif