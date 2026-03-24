#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "vad.h"
#include "pav_analysis.h"

const float FRAME_TIME = 10.0F; /* ms */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT", "MS", "MV"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

typedef struct {
  float p;   /* potencia en dB */
} Features;

Features compute_features(const float *x, int N) {
  Features feat;
  feat.p = compute_power(x, N);
  return feat;
}

VAD_DATA *vad_open(float rate) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state       = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length  = rate * FRAME_TIME * 1e-3;
  vad_data->sum_power     = 0.0F;
  vad_data->init_count    = 0;
  vad_data->frame_count   = 0;
  vad_data->k0 = vad_data->k1 = vad_data->k2 = 0.0F;
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /* las tramas maybe_silence al final las tratamos como silencio */
  VAD_STATE state = vad_data->state;
  if (state == ST_MAYBE_SILENCE) state = ST_SILENCE;
  if (state == ST_MAYBE_VOICE)   state = ST_VOICE;
  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

VAD_STATE vad(VAD_DATA *vad_data, float *x, float alpha0) {

  Features f = compute_features(x, vad_data->frame_length);
  vad_data->last_feature = f.p;

  switch (vad_data->state) {

  case ST_INIT:
    /* Acumulamos potencia durante N_INIT tramas para estimar k0 */
    vad_data->sum_power += pow(10.0F, f.p / 10.0F);
    vad_data->init_count++;

    if (vad_data->init_count >= N_INIT) {
      /* k0 = potencia media inicial en dB */
      vad_data->k0 = 10.0F * log10f(vad_data->sum_power / N_INIT);
      vad_data->k1 = vad_data->k0 + ALPHA1;
      vad_data->k2 = vad_data->k1 + ALPHA2;
      vad_data->state = ST_SILENCE;
    }
    return ST_SILENCE; /* durante la fase de init, consideramos que no hay voz */
    break;

  case ST_SILENCE:
    if (f.p > vad_data->k1) {
      vad_data->state = ST_MAYBE_VOICE;
      vad_data->frame_count = 1;
    }
    break;

  case ST_MAYBE_VOICE:
    if (f.p > vad_data->k1) {
      vad_data->frame_count++;
      if (vad_data->frame_count >= MIN_VOICE_F) {
        vad_data->state = ST_VOICE;
        vad_data->frame_count = 0;
      }
    } else {
      /* no aguanta: volvemos a silencio */
      vad_data->state = ST_SILENCE;
      vad_data->frame_count = 0;
    }
    break;

  case ST_VOICE:
    if (f.p < vad_data->k1) {
      vad_data->state = ST_MAYBE_SILENCE;
      vad_data->frame_count = 1;
    }
    break;

  case ST_MAYBE_SILENCE:
    if (f.p < vad_data->k1) {
      vad_data->frame_count++;
      if (vad_data->frame_count >= MIN_SIL_F) {
        vad_data->state = ST_SILENCE;
        vad_data->frame_count = 0;
      }
    } else {
      /* recupera potencia: volvemos a voz */
      vad_data->state = ST_VOICE;
      vad_data->frame_count = 0;
    }
    break;

  case ST_UNDEF:
    break;
  }

  /* Hacia afuera solo exportamos S o V (nunca los estados maybe) */
  if (vad_data->state == ST_SILENCE ||
      vad_data->state == ST_MAYBE_SILENCE)
    return ST_SILENCE;
  else if (vad_data->state == ST_VOICE ||
           vad_data->state == ST_MAYBE_VOICE)
    return ST_VOICE;
  else
    return ST_UNDEF;
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "state=%s\tp=%.2f\tk0=%.2f\tk1=%.2f\tk2=%.2f\n",
          state_str[vad_data->state],
          vad_data->last_feature,
          vad_data->k0, vad_data->k1, vad_data->k2);
}