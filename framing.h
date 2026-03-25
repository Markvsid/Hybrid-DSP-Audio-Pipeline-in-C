#ifndef FRAMING_H
#define FRAMING_H

typedef struct {
    const double *input;
    unsigned input_len;
    unsigned frame_len;
    unsigned hop_len;
    unsigned fft_len;
    unsigned fs; //sampling frequency
    unsigned index; //frame index
    unsigned sample_position; //input start
} frame;   

unsigned frame_len_compute(unsigned fs, double frame_ms);
unsigned next_pow_2(unsigned n);
unsigned hop_len_compute(unsigned frame_len);
int frame_source_init(const double *input, frame *f, unsigned input_len, unsigned frame_len, unsigned hop_len);
void frame_source_reset(frame *f);
unsigned frame_len_compute(unsigned fs, double frame_ms);
int get_next_frame(frame *f, double *frame_out);



#endif
