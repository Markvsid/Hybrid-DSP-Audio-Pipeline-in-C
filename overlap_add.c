#include "overlap_add.h"
#include <stddef.h>
#include <stdlib.h>

int ola_init(ola *f, unsigned frame_len, unsigned hop_len){ //init can only be called once in a row
    if (f == NULL || frame_len == 0 || hop_len == 0 || hop_len > frame_len) {
        return -1;
    }

    if (f->buffer != NULL) { // caller must zero-initialize struct before first use; re-init without ola_free is rejected
        return -1;
    }

    f->buffer = calloc(frame_len, sizeof(double));
    if (f->buffer == NULL) {
        return -1;
    }

    f->frame_len = frame_len;
    f->hop_len = hop_len;

    return 0;
}

int ola_process_frame(ola *f, const double *frame_in, double *hop_out) {
    if (f == NULL || frame_in == NULL || f->buffer == NULL || f->frame_len == 0
          || f->hop_len == 0 || f->hop_len > f->frame_len || hop_out == NULL) {
        return -1;
    }

    for (unsigned i = 0; i < f->frame_len; i++) {
        f->buffer[i] += frame_in[i]; 
    }

    for (unsigned i = 0; i < f->hop_len; i++) {
        hop_out[i] = f->buffer[i];
    }

    for (unsigned i = 0; i < f->frame_len - f->hop_len; i++) {
        f->buffer[i] = f->buffer[i + f->hop_len];
    }

    for (unsigned i = f->frame_len - f->hop_len; i < f->frame_len; i++) {
        f->buffer[i] = 0.0;
    }

    return 0;
}

void ola_free(ola *f) {
    if (f == NULL) {
        return;
    }
    
    if (f->buffer != NULL) {
        free(f->buffer);
    }

    f->buffer = NULL;
    f->frame_len = 0;
    f->hop_len = 0;

}
