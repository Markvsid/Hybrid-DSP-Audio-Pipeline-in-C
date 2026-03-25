#include "window.h"
#include <stddef.h>
#include <math.h>

#define PI 3.14159265358979323846

void make_hann_window(double *w, unsigned N) {

    if(w == NULL || N == 0) {
        return;
    }

    if(N==1) {
        w[0] = 1;
        return;
    }
 
    for(unsigned n=0; n<N; n++) {
        w[n] = 0.5*(1.0 - cos((2.0 * PI * n) / ((double)(N-1))));
    }
}

void apply_window(double *x, const double *w, unsigned N) {

    if(x == NULL || w == NULL || N == 0) {
        return;
    }
    for(unsigned n=0; n<N; n++) {
        x[n] *= w[n];
    }
}
