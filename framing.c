//25 ms frame
//12.5 ms hop
//50% overlap
#include "framing.h"
#include <math.h>
#include <stddef.h>

//int frame_len = (unsigned)(.025 * fs);
//int hop_len = frame_len / 2;

    unsigned frame_len_compute(unsigned fs, double frame_ms) {
        if (fs == 0 || frame_ms <=0) {
            return 0; //frame_len_compute is the only exception to return 1 on failure rule
        }

        frame_ms /= 1000;
        unsigned samples = round(fs * frame_ms);

        if (samples % 2 !=0) {
            samples++;
        }

        if (samples == 0) {
            return 0;
        }
        
        return samples;
    }

unsigned next_pow_2(unsigned n) {
    if (n == 0) {
        return 0;
    }

    unsigned p = 1;
    while(p < n) {
        p <<= 1;
    }
    return p;

}

unsigned hop_len_compute(unsigned frame_len) {
    unsigned hop_len = frame_len / 2; //50% overlap
    return hop_len;
}

int frame_source_init(const double *input, frame *f, unsigned input_len, unsigned frame_len, unsigned hop_len) {
    if (!input || !f || input_len == 0 || frame_len == 0 || hop_len == 0 || hop_len > frame_len) {
        return -1;
    }

    f->input = input;
    f->input_len = input_len;
    f->frame_len = frame_len;
    f->hop_len = hop_len;

    f->sample_position = 0;
    f->index = 0; //frame index for debugging and data

    f->fs = 0;
    f->fft_len = next_pow_2(frame_len);


    return 0;
}

int get_next_frame(frame *f, double *frame_out) {
    if (f == NULL || frame_out == NULL) {
        return -1;
    }

    if (f->input == NULL) {
        return -1;
    }

    if (f->frame_len == 0 || f->hop_len == 0 || f->fft_len == 0) {
        return -1;
    }

    if (f->sample_position >= f->input_len) {   
        return -1;
    }

    if (f->fft_len < f->frame_len) {
        return -1;
    }

    for (unsigned i = 0; i < f->fft_len; i++) {
        frame_out[i] = 0.0;
    }

    for (unsigned i=0; i < f->frame_len; i++) {
            if (f->sample_position + i < f->input_len) {
                frame_out[i] = f->input[f->sample_position + i];
            } else {
                break;
        }
    }

    f->sample_position += f->hop_len;
    f->index += 1;
    return 0; 

}

void frame_source_reset(frame *f) {
    if (f == NULL) {
        return;
    }

    f->sample_position = 0;
    f->index = 0;
}



    


