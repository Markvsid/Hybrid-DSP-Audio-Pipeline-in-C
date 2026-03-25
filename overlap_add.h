#ifndef OVERLAP_ADD
#define OVERLAP_ADD

typedef struct {
    double *buffer; //accumulation buffer
    unsigned frame_len;
    unsigned hop_len;
} ola;


int ola_init(ola *f, unsigned frame_len, unsigned hop_len);
int ola_process_frame(ola *f, const double *frame_in, double *hop_out);
void ola_free(ola *f);

#endif
