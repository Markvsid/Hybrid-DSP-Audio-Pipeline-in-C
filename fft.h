#ifndef FFT_H
#define FFT_H

typedef struct { 
    double real; 
    double imaginary;
} cplx;


static inline cplx cplx_add (cplx a, cplx b) 
{
    cplx result;
    result.real = a.real + b.real;
    result.imaginary = a.imaginary + b.imaginary;
return result;
}

static inline cplx cplx_sub (cplx a, cplx b)
{
    cplx result;
    result.real = a.real - b.real;
    result.imaginary = a.imaginary - b.imaginary;
    return result;
}

static inline cplx cplx_mult (cplx a, cplx b)
{
    cplx result;
    result.real = a.real * b.real - a.imaginary * b.imaginary;
    result.imaginary = a.real * b.imaginary + a.imaginary * b.real;
    return result;

}

static inline cplx cplx_div (cplx a, cplx b)
{
    cplx result; 
    result.real = 
        (a.real * b.real + a.imaginary * b.imaginary) / 
        ((b.real * b.real) + (b.imaginary * b.imaginary));
    result.imaginary =
        (a.imaginary * b.real - a.real * b.imaginary) / 
        ((b.real * b.real) + (b.imaginary * b.imaginary));
    return result;

}

static inline cplx cplx_conj (cplx a)
{
    cplx result;
    result.real = a.real;
    result.imaginary = -a.imaginary;
    return result;

}

static inline cplx cplx_scale (cplx a, double s)
{
    cplx result;
    result.real = s * a.real;
    result.imaginary = s * a.imaginary;     
    return result;

}

int fft_forward(cplx *x, unsigned N);
int fft_inverse(cplx *x, unsigned N);



#endif
