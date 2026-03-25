#include "fft.h"
#include <math.h>
#include <stdio.h> //for printing
#include <stddef.h>

//stft parameters fixed in time
//frame = 32ms
//hop = 16ms
#define PI 3.14159265358979323846
/*
static void print_signal(const char *label, cplx *x, unsigned int N) {
    printf("%s\n", label);
    for (unsigned int i = 0; i < N; i++) {
        printf("[%u] = %8.4f %+8.4fj\n", i, x[i].real, x[i].imaginary);
    }
    printf("\n");
}
*/
static int is_pow2 (unsigned N) {

    return N && !(N & (N-1));
}

static inline void swap (cplx *a, cplx *b) {
    cplx temp = *a;
    *a = *b;
    *b = temp;
}

static unsigned log_2(unsigned N) {
    unsigned r=0;
    while (N >>= 1) {
        r++;
    }
    return r;
}

static unsigned reverse_bits (unsigned x, unsigned bits) {
    unsigned r=0;
    for (unsigned i=0; i<bits; i++) {
        r = (r << 1) | (x & 1);
        x >>= 1;
    }   
    return r;
}

static void reverse_bit_permute (unsigned N, cplx *x) {
    unsigned bits = log_2(N);

    unsigned j = 0;
    for(unsigned i=0; i<N; i++) {
        j = reverse_bits(i,bits);
        if (j>i) {
            swap(&x[i], &x[j]);
        }
    }
}

int fft_forward(cplx *x, unsigned N) {

    if (x==NULL) {
        return -1;
    }   

    if (!is_pow2(N)) {
        printf("Error, N is not a power of 2\n");
        return -1;
    }


reverse_bit_permute(N, x);

for(unsigned m=2; m<=N; m<<=1) {

    unsigned half = m>>1;

    double theta = (-2.0 * PI / m);
    cplx wm = {cos(theta), sin(theta)};

    for (unsigned k=0;k<N; k+=m) {

        cplx w;
        w.real = 1.0;
        w.imaginary = 0.0;

        for(unsigned j=0; j<half; j++) {

            cplx u = x[k+j];
            cplx v = x[k+j+half];
            cplx t = cplx_mult(v, w);
            
            x[k+j] = cplx_add(u,t);
            x[k+j+half] = cplx_sub(u,t);

            w = cplx_mult(w,wm);
        }
    }

}


    return 0;

}

int fft_inverse(cplx *x, unsigned N) {

    if (x==NULL) {
        return -1;
    }

    if (!is_pow2(N)) {
        printf("Error, N is not a power of 2\n");
        return -1;
    }

reverse_bit_permute(N, x);

for(unsigned m=2; m<=N; m<<=1) {

    unsigned half = m>>1;

    double theta = (2.0 * PI / m);
    cplx wm = {cos(theta), sin(theta)};

    for (unsigned k=0;k<N; k+=m) {

        cplx w;
        w.real = 1.0;
        w.imaginary = 0.0;

        for(unsigned j=0; j<half; j++) {

            cplx u = x[k+j];
            cplx v = x[k+j+half];
            cplx t = cplx_mult(v, w);
            
            x[k+j] = cplx_add(u,t);
            x[k+j+half] = cplx_sub(u,t);

            w = cplx_mult(w,wm);
        }
    }

}

    for(unsigned i=0; i<N; i++) {
        x[i].real /= (double)N;
        x[i].imaginary /= (double)N;

    }   

    return 0;
}
/*
int main(void) {

    unsigned N = 128;

    double samples[128] = {
    1.600000,  1.505050,  1.186599,  0.708248,  0.171049, -0.315760, -0.665911, -0.838681,
   -0.847244, -0.752524, -0.641421, -0.598550, -0.679388, -0.892920, -1.201557, -1.538842,
   -1.834925, -2.041497, -2.146986, -2.178945, -2.191778, -2.244306, -2.376811, -2.593999,
   -2.860074, -3.107722, -3.255789, -3.229451, -2.978650, -2.492592, -1.807413, -0.999014,
   -0.165685,  0.594893,  1.205055,  1.622152,  1.846546,  1.920388,  1.914824,  1.911801,
    1.982843,  2.170384,  2.475540,  2.857254,  3.240536,  3.535859,  3.664365,  3.579195,
    3.278231,  2.807129,  2.249842,  1.710389,  1.292893,  1.082874,  1.131026,  1.443674,
    1.978950,  2.652664,  3.351600,  3.952785,  4.345481,  4.451819,  4.242948,  3.747359,
    3.045957,  2.254827,  1.501537,  0.900639,  0.532081,  0.425593,  0.554846,  0.844782,
    1.190207,  1.479627,  1.621320,  1.564619,  1.312132,  0.919713,  0.483592,  0.118834,
   -0.071797, -0.026127,  0.250842,  0.691525,  1.182117,  1.591458,  1.804251,  1.749934,
    1.421775,  0.879338,  0.233037, -0.383075, -0.841982, -1.051714, -0.976297, -0.646854,
   -0.157387,  0.362927,  0.782843,  1.002930,  0.980785,  0.739222,  0.357019, -0.060877,
   -0.407107, -0.598878, -0.598292, -0.420106, -0.126653,  0.192585,  0.440933,  0.537545,
    0.437675,  0.145005, -0.281626, -0.739900, -1.106852, -1.271621, -1.164173, -0.776429,
   -0.167705,  0.556110,  1.255261,  1.786469,  2.033663,  1.935686,  1.503039,  0.817228
};   

cplx x[128] = {0};
x[0].real = 1.0; */
/*
cplx x[128];
for (int n = 0; n < 128; n++) {
    x[n].real = samples[n];
    x[n].imaginary = 0.0;
} */

  /*  print_signal("Input:", x, N);

    if (fft_forward(x, N) !=0) {
        fprintf(stderr, "FFT error: N must be a power of 2 and x must not be NULL.\n");
        return 1;
    }

    print_signal("FFT Output:", x, N); 
    return 0;
}

*/
