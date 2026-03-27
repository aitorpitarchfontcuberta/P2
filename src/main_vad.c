#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sndfile.h>

#include "vad.h"
#include "vad_docopt.h"

#define DEBUG_VAD 0x1

int main(int argc, char *argv[]) {
  int verbose = 0;

  SNDFILE *sndfile_in, *sndfile_out = 0;
  SF_INFO sf_info;
  FILE *vadfile;
  int n_read = 0, i;

  VAD_DATA *vad_data;
  VAD_STATE state, last_state;

  float *buffer, *buffer_zeros;
  int frame_size;
  float frame_duration;
  unsigned int t, last_t;

  char *input_wav, *output_vad, *output_wav;

  DocoptArgs args = docopt(argc, argv, /* help */ 1, /* version */ "2.0");

  verbose    = args.verbose ? DEBUG_VAD : 0;
  input_wav  = args.input_wav;
  output_vad = args.output_vad;
  output_wav = args.output_wav;
  float alpha0 = atof(args.alpha0);

  if (input_wav == 0 || output_vad == 0) {
    fprintf(stderr, "%s\n", args.usage_pattern);
    return -1;
  }

  if ((sndfile_in = sf_open(input_wav, SFM_READ, &sf_info)) == 0) {
    fprintf(stderr, "Error opening input file %s (%s)\n", input_wav, strerror(errno));
    return -1;
  }

  if (sf_info.channels != 1) {
    fprintf(stderr, "Error: the input file has to be mono: %s\n", input_wav);
    return -2;
  }

  if ((vadfile = fopen(output_vad, "wt")) == 0) {
    fprintf(stderr, "Error opening output vad file %s (%s)\n", output_vad, strerror(errno));
    return -1;
  }

  if (output_wav) {
    if ((sndfile_out = sf_open(output_wav, SFM_WRITE, &sf_info)) == 0) {
      fprintf(stderr, "Error opening output wav file %s (%s)\n", output_wav, strerror(errno));
      return -1;
    }
  }

  vad_data = vad_open(sf_info.samplerate, alpha0, ALPHA1, ALPHA2);
  frame_size   = vad_frame_size(vad_data);
  buffer       = (float *) malloc(frame_size * sizeof(float));
  buffer_zeros = (float *) malloc(frame_size * sizeof(float));
  for (i = 0; i < frame_size; ++i) buffer_zeros[i] = 0.0F;

  frame_duration = (float) frame_size / (float) sf_info.samplerate;
  last_state = ST_SILENCE;

  for (t = last_t = 0; ; t++) {
    if ((n_read = sf_read_float(sndfile_in, buffer, frame_size)) != frame_size) break;

    state = vad(vad_data, buffer);
    if (verbose & DEBUG_VAD) vad_show_state(vad_data, stdout);

    /* Escribir en output_wav: ceros si silencio, señal original si voz */
    if (sndfile_out != 0) {
      if (state == ST_VOICE)
        sf_write_float(sndfile_out, buffer, frame_size);
      else
        sf_write_float(sndfile_out, buffer_zeros, frame_size);
    }

    /* Solo imprimimos cuando cambia el estado, y nunca imprimimos UNDEF */
    if (state != last_state) {
      if (last_state != ST_UNDEF && t != last_t)
        fprintf(vadfile, "%.5f\t%.5f\t%s\n",
                last_t * frame_duration,
                t * frame_duration,
                state2str(last_state));
      last_state = state;
      last_t = t;
    }
  }

  /* Última etiqueta: usar vad_close para resolver estados maybe pendientes */
  state = vad_close(vad_data);
  if (state == ST_UNDEF) state = ST_SILENCE; /* fallback seguro */
  if (last_state != ST_UNDEF && t != last_t)
    fprintf(vadfile, "%.5f\t%.5f\t%s\n",
            last_t * frame_duration,
            t * frame_duration + n_read / (float) sf_info.samplerate,
            state2str(state));

  free(buffer);
  free(buffer_zeros);
  sf_close(sndfile_in);
  fclose(vadfile);
  if (sndfile_out) sf_close(sndfile_out);
  return 0;
}