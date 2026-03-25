#ifndef AUDIO_IO_H
#define AUDIO_IO_H

typedef struct {
    unsigned fs;
    unsigned num_samples;
    double *samples;
} audio;

int read_wav_mono(const char *filename, audio *buf);
int write_wav_mono(const char *filename, const audio *buf);
void free_audio_buffer(audio *buf);

#endif
