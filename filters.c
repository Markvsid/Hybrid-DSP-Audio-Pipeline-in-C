//time domain fir and iir ->high pass butterworth   (order 2, cuttof fc 80hz, ) biquad
//iir butterworth -> possibly compare with chebyshev or make optional
//iir low and high pass -> add a user optional fir and compare runtime

//variable sample rate fs
//N (order) = 2
//cutoff frequency fc = 80 Hz

#include "filters.h"
#include <math.h>
#include "audio_io.h"
#include "window.h"
#define FC 80.0 //cutoff frequency fc hard coded
#define PI 3.14159265358979323846
#define SQRT2 1.4142135623730951
#define TAPS 101 //linear phase fir delay = (M-1)/2

int butterworth_highpass_init(iir_data *x, double fs) {
    if (!x || fs <= 0.0 || FC >= fs / 2.0) {
        return -1;
    }

    const double k = tan(PI * FC / fs);
    const double norm = 1.0 / (1 + SQRT2 * k + (k * k));

    x->b0 = norm;
    x->b1 = -2.0 * norm;
    x->b2 = norm;

    x->a1 = 2.0 * ((k * k) - 1.0) * norm;
    x->a2 = (1 - (SQRT2 * k) + (k * k)) * norm;

    x->x1 = 0.0;
    x->x2 = 0.0;

    x->y1 = 0.0;
    x->y2 = 0.0;

    return 0;

}

//y[z]=b0​x[n]+b1​x[n−1]+b2​x[n−2]−a1​y[n−1]−a2​y[n−2]
double butterworth_highpass(iir_data *f, double x) {
    if (!f) return -1;
    double z = f->b0 * x
        + f->b1 * f->x1
        + f->b2 * f->x2
        - f->a1 * f->y1
        - f->a2 * f->y2;

        f->x2 = f->x1;
        f->x1 = x;
        f->y2 = f->y1;
        f->y1 = z;


    return z;

}   

//fir implementation
//coefficients initialized with HANN window for better wiener filtering in frequency domain
//y[n] = h[0]x[n] + h[1]x[n-1] + ... + h[M-1]x[n-(M-1)] 
int fir_highpass_init(fir_data *f ,double fs){
    if (!f || fs <= 0.0 || FC >= fs / 2.0) {
        return -1;
    }

    int center = ((TAPS - 1) / 2);
    double fn = FC / fs;
    int k=0;


    for (int n=0; n<TAPS; n++) {
        k = n - center;
        double h_lp = 0; //low pass coefficients

        if (k==0) {
            h_lp = 2.0 * fn;
        }  
        else {
            h_lp = (sin(2.0 * PI * fn * k) / (PI * k));
        }

    //spectral inversion hHP​[n]=δ[n−center]−hLP​[n]
    if (n==center) {
        f->h[n] = 1.0 - h_lp;

    }
    else {
        f->h[n] = -h_lp;
    }
}
    double window[TAPS];
    make_hann_window(window, TAPS);
    apply_window(f->h, window, TAPS);

    for (int i=0; i<TAPS; i++) {
        f->x_hist[i] = 0.0;
    }

    
    return 0;
}

//validate input
//compute coefficients
//apply window
//zero state buffer
//return SUCCESSS

double fir_highpass(fir_data *f, double x) { //x is current input sample
    if (!f) {
        return 0.0;
    }

    for (int i = TAPS - 1; i>0; i--) {
        f->x_hist[i] = f->x_hist[i-1];
    }

    f->x_hist[0] = x;


    double y_n = 0.0;
    for (int i=0; i<TAPS; i++) {
        y_n += f->h[i] * f->x_hist[i];
    }

    return y_n;

}
