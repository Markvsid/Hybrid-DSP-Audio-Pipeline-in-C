#ifndef WINDOW_H
#define WINDOW_H

void make_hann_window(double *w, unsigned N);
void apply_window(double *x, const double *w, unsigned N);//N = length, W = window, X = chunk


#endif
