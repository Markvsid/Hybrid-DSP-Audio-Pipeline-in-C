//16 bit PCM only, variable sample rate
// ****************** RETURN CONVENTION: ************************
//       Nonzero for FAILURE
//       0 for success
//***************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "audio_io.h"
#include "filters.h"
#include "framing.h"
#include "window.h"
#include "fft.h"
#include "spectral.h"
#include "overlap_add.h"

#define NOISE_INIT 10

int main (int argc, char *argv[]) {
    int status = -1;
    audio buf = {0};
    audio out = {0};
    frame f = {0};
    ola ola_state = {0};
    wiener_data st = {0};

    double *samples = NULL;
    double *win = NULL;
    double *frame_buf = NULL;
    double *hop_out = NULL;
    double *out_samples = NULL;
    double *zero_frame = NULL;
    cplx *x = NULL;
    
    if (argc != 3) {
        printf("Format: %s input.wav output.wav\n", argv[0]);
        goto cleanup;
    }

    char filt_arg[3];
    printf("Select filter type: '1' = fir, '2' = iir\n");
    if (fgets(filt_arg, sizeof(filt_arg), stdin) == NULL) {
        printf("Failed to read filter choice\n");
        goto cleanup;
    }

    if (filt_arg[0] != '1' && filt_arg[0] != '2') {
        printf("Invalid filter choice\n");
        goto cleanup;
    }

    if (read_wav_mono(argv[1], &buf) != 0) {
        printf("File read failed\n");
        goto cleanup;
    }
    printf("File read success\n");
    unsigned fs = buf.fs;

    samples = (malloc(buf.num_samples * sizeof(double)));
    if (samples == NULL) {
        printf("Allocation for samples failed\n");
        goto cleanup;
    }
    for (unsigned i = 0; i < buf.num_samples; i++) {
        samples[i] = buf.samples[i]; //modularity and easier read
    }

    if (filt_arg[0] == '2') {
        iir_data hp;
        if ((butterworth_highpass_init(&hp, fs) != 0)) {
            printf("iir filter init failed\n");
            goto cleanup;
        }
        for (unsigned i = 0; i < buf.num_samples; i++) {
        samples[i] = butterworth_highpass(&hp, samples[i]);
        }
    }  
    
    if (filt_arg[0] == '1') {
        fir_data fir_hp;
        if (fir_highpass_init(&fir_hp, fs) != 0) {
            printf("fir filter failed\n");
            goto cleanup;
        }
        for (unsigned i = 0; i < buf.num_samples; i++) {
        samples[i] = fir_highpass(&fir_hp, samples[i]);
        }
    }

    double frame_ms = 25.0;
    unsigned frame_len = frame_len_compute(fs, frame_ms);
    unsigned hop_len = hop_len_compute(frame_len);

    if (frame_len == 0) { //frame_len_compute returns frame length, so nonzero does not mean error
        printf("Invalid frame length\n"); //^because 1 is a valid frame len
        goto cleanup;
    }

    win = malloc(frame_len * sizeof(double));
    if (win == NULL) {
        printf("window allocation failed\n");
        goto cleanup;
    }

    make_hann_window(win, frame_len);

    if (frame_source_init(samples, &f, buf.num_samples, frame_len, hop_len) != 0) {
        printf("Frame initialization failed\n");
        goto cleanup;
    }

    frame_buf = malloc(frame_len * sizeof(double));
    if (frame_buf == NULL) {
        printf("frame buffer allocation failed\n");
        goto cleanup;
    }

    x = malloc(f.fft_len * sizeof(cplx));
        if (x == NULL) {
        printf("FFT buffer allocate failed\n");
        goto cleanup;
    }

    if (wiener_init(&st, f.fft_len, fs) != 0) {
        printf("massive WIENER failure\n");
        goto cleanup;
    }

    hop_out = malloc(f.hop_len * sizeof(double));
    if (hop_out == NULL) {
        printf("Failure allocating hop_out\n");
        goto cleanup;
    }

   if (ola_init(&ola_state, f.frame_len, f.hop_len) != 0) {
        printf("ola init failure\n");
        goto cleanup;
    }

    out_samples = malloc((buf.num_samples + f.frame_len) * sizeof(double));
    if (out_samples == NULL) {
        printf("error allocating out_samples\n");
        goto cleanup;
    }

    zero_frame = calloc(ola_state.frame_len, sizeof(double));
    if (zero_frame == NULL) {
        printf("zero frame allocation failure\n");
        goto cleanup;
    }

    unsigned out_index = 0;
    unsigned wiener_init_index = 0;

    while (get_next_frame(&f, frame_buf) == 0) { // * * * * * * * * * * * * while loop * * * * * * * * * * * * * *
        apply_window(frame_buf, win, f.frame_len);
        for (unsigned i = 0; i < f.fft_len; i++) {
            if (i < f.frame_len) {
                x[i].real = frame_buf[i];
            } else {
                x[i].real = 0.0;
            }
            x[i].imaginary = 0.0;
        }
        if (fft_forward(x, f.fft_len) != 0) {
            printf("FFT failure\n");
            goto cleanup;
        }

        if (wiener_init_index < NOISE_INIT) {
            if (wiener_estimate_noise(&st, x) != 0) {
                printf("MASSIVE WIENER estimation failure\n");
                goto cleanup;
            }
            wiener_init_index += 1;
        } else {
            if (wiener_filter_frame(&st, x) != 0) {
            printf("WIENER filter frame failure\n");
            goto cleanup;
            } 
        }

        if (fft_inverse(x, f.fft_len) != 0) {
            printf("IFFT failure\n");
            goto cleanup;
        }

        for (unsigned i = 0; i < f.frame_len; i++) {
            frame_buf[i] = x[i].real;
        }

        if (ola_process_frame(&ola_state, frame_buf, hop_out) != 0) {
            printf("ola processing failure\n");
            goto cleanup;
        }

        for (unsigned i = 0; i < f.hop_len; i++) {
            if (out_index < buf.num_samples + f.frame_len) {
            out_samples[out_index++] = hop_out[i];
            }
        }


    }

    unsigned flush_calls = (ola_state.frame_len - ola_state.hop_len) / ola_state.hop_len;
    for (unsigned i = 0; i < flush_calls; i++) {
        if (ola_process_frame(&ola_state, zero_frame, hop_out) != 0) {
            goto cleanup;
        }
    

        for (unsigned j = 0; j < f.hop_len; j++) {
            if (out_index < buf.num_samples + f.frame_len) {
                out_samples[out_index++] = hop_out[j];
            }
        }
    }

    out.fs = buf.fs;
    out.num_samples = out_index;
    out.samples = out_samples;
    out_samples = NULL;

    if (write_wav_mono(argv[2], &out) != 0) {
        printf("File write failed\n");
        goto cleanup;
    }

    status = 0;
cleanup:
    ola_free(&ola_state);
    free(frame_buf);
    free(win);
    free_audio_buffer(&buf);
    free_audio_buffer(&out); //frees out.samples
    wiener_free(&st);
    free(x);
    free(samples);
    free(hop_out);
    free(zero_frame);

    return status;
}

