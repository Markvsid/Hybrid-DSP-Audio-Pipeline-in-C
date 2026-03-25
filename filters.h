#ifndef FILTERS_H
#define FILTERS_H

#include "audio_io.h"

#define TAPS 101 //linear phase fir delay = (M-1)/2

typedef struct {
    double b0, b1, b2;
    double a1, a2;
    double x1, x2;
    double y1, y2;

} iir_data;

typedef struct {
    double h[TAPS];     //coefficients
    double x_hist[TAPS]; //input history
} fir_data;

int butterworth_highpass_init(iir_data *x, double fs); 
double butterworth_highpass(iir_data *f, double x);

int fir_highpass_init(fir_data *f ,double fs);
double fir_highpass(fir_data *h, double x);




#endif
