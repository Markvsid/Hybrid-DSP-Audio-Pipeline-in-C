    //short time spectral wiener filter frequency domain
    //STFT-based Wiener filter on each frame, using per-bin power estimates
    //per-bin power wiener filter
    //running average noise estimate

    #include "spectral.h"
    #include "fft.h"
    #include <stddef.h>
    #include <stdlib.h>

    #define GAIN_FLOOR 0.2
    #define ALPHA 0.4
    #define GAIN_SMOOTH 0.8

    int wiener_init(wiener_data *st, unsigned fft_len, unsigned fs) { //wiener_init cannot be called twice in a row
        if (st == NULL || fft_len == 0 || fs == 0) {
            return -1;
        }

        st->fft_len = 0;
        st->fs = 0;
        st->noise_psd = NULL;
        st->noise_frames_accumulated = 0;
        st->prev_gain = NULL;

        st->noise_psd = calloc(fft_len, sizeof(double));
        if (st->noise_psd == NULL) {
            return -1;
        }

        st->prev_gain = calloc(fft_len, sizeof(double));
        if (st->prev_gain == NULL) {
            free(st->noise_psd);
            st->noise_psd = NULL;
            return -1;
        }

        for (unsigned i = 0; i < fft_len; i++) {
            st->prev_gain[i] = 1.0;
        }

        st->fft_len = fft_len;
        st->fs = fs;

        return 0;
    }

    void wiener_free(wiener_data *st) {
        if (st == NULL) {
            return;
        }

        if (st->noise_psd != NULL) {
            free(st->noise_psd);
        }

        st->noise_psd = NULL; //eliminate dangling pointer
        st->noise_frames_accumulated = 0;
        st->fft_len = 0;
        st->fs = 0;

    }

    int wiener_estimate_noise(wiener_data *st, const cplx *Y) {
        if (st == NULL || Y == NULL || st->noise_psd == NULL || st->fft_len == 0 ) {
            return -1;
        }

        //unsigned n = st->noise_frames_accumulated;

        for (unsigned i = 0; i < st->fft_len; i++) {
            double power = Y[i].real * Y[i].real + Y[i].imaginary * Y[i].imaginary;

    //        if (st->noise_frames_accumulated == 0) {
    //            st->noise_psd[i] = power;
    //        } else {
    //        st->noise_psd[i] = (st->noise_frames_accumulated * st->noise_psd[i] + power) / (st->noise_frames_accumulated + 1); //becomes expectation of avg noise power
            st->noise_psd[i] = ALPHA * st->noise_psd[i] + (1 - ALPHA) * power;
    //        }
    //        if (st->noise_psd[i] < NOISE_PSD_FLOOR) {
    //           st->noise_psd[i] = NOISE_PSD_FLOOR;
    //        }
        }

        st->noise_frames_accumulated +=1;
        return 0;
    }
                                                            //first attempt, typical wiener gain
    int wiener_filter_frame(wiener_data *st, cplx *Y) {             
        if (st == NULL || Y == NULL || st->noise_psd == NULL || st->fft_len ==0 || st->noise_frames_accumulated == 0 ) {
            return -1;
        }

        for (unsigned i = 0; i < st->fft_len; i++ ) {

            double py = 0;
            double pn = 0; //psd estimate
            double px = 0; //estimate
            double H = 0; //wiener gain
            double H_raw = 0; // used for gain smoothing

            py = Y[i].real * Y[i].real + Y[i].imaginary * Y[i].imaginary; //instantaneous observed power
            pn = st->noise_psd[i];
                /*
            if (py - pn > 0) {
                px = py - pn;
            } else {
                px = 0; //power cannot be negative
            }

            if (py > 1e-12) {
                H = px / py; //gain compute
            } else {
                H = GAIN_FLOOR; //avoid divide by 0 
            }

            if (H > 1) {
                H = 1;
            }

            if (H < GAIN_FLOOR ) {
                H = GAIN_FLOOR; //gain floor 
            }
                    */

            px = py - pn;

            if (py > 1e-12) {
                H_raw = px / py; //gain compute
            } else {
                H_raw = GAIN_FLOOR; //avoid divide by 0 
            }

            if (H_raw > 1.0) {
                H_raw = 1.0;
            }
            
            if (H_raw < GAIN_FLOOR) {
                H_raw = GAIN_FLOOR;
            }

            H = GAIN_SMOOTH * st->prev_gain[i] + (1.0 - GAIN_SMOOTH) * H_raw;
            st->prev_gain[i] = H;


            Y[i].real *= H;
            Y[i].imaginary *= H;

        }

        return 0;

    } 

/*
    int wiener_filter_frame(wiener_data *st, cplx *Y) { //exponential smoothing
        if (st == NULL || Y == NULL || st->noise_psd == NULL || st->fft_len ==0 || st->noise_frames_accumulated == 0 ) {
            return -1;
        }

        for (unsigned i = 0; i < st->fft_len; i++) {
            double py = 0;
            double H = 0; //wiener gain P hat[k]
        
            py = Y[i].real * Y[i].real + Y[i].imaginary * Y[i].imaginary;    
            H = ALPHA * st->prev_gain[i] + (1 - ALPHA) * py;

            if (H > 1) {
                H = 1;
            }

            if (H < GAIN_FLOOR ) {
                H = GAIN_FLOOR; //gain floor = 0.05
            }

            Y[i].real *= H;
            Y[i].imaginary *= H;





        }
        return 0;

    }
*/
    
