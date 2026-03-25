#ifndef SPECTRAL_H
#define SPECTRAL_H

#include "fft.h"
typedef struct {
    unsigned fft_len;
    unsigned fs;
    double *noise_psd; //average noise power spectral density
    unsigned noise_frames_accumulated;
    double *prev_gain;
} wiener_data;

int wiener_init(wiener_data *st, unsigned fft_len, unsigned fs);
void wiener_free(wiener_data *st);

int wiener_estimate_noise(wiener_data *st, const cplx *Y);
int wiener_filter_frame(wiener_data *st, cplx *Y);




#endif
