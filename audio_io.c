//sample rate acquired from wav file
//WAV uses little endianness

#include "audio_io.h"
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

int read_wav_mono(const char *filename, audio *buf) {
    if (filename == NULL || buf == NULL) {
        return -1;
    }

    buf->fs = 0;
    buf->num_samples = 0;
    buf->samples = NULL;

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("Error opening file\n");
        return -1;
    }

    char chunk_id[4];   
    uint32_t chunk_size;
    char format[4];
//fread(void *ptr, size_t size, size_t count, FILE *stream);
    if (fread(chunk_id, 1, 4, fp) != 4) { //read 4 bytes
        fclose(fp);
        printf("chunk_id read failure\n");
        return -1;
    }

    if (fread(&chunk_size, sizeof(uint32_t), 1, fp) != 1) { //fread advances file pointer implicitly
        fclose(fp);
        printf("Chunk_size read failure\n");
        return -1;
    }

    if (fread(format, 1, 4, fp) != 4) {
        fclose(fp);
        printf("Format reat failure\n");
        return -1;
    }

    if (memcmp(chunk_id, "RIFF", 4) != 0|| memcmp(format, "WAVE", 4) != 0) {
        fclose(fp);
        printf("Format not RIFF or file not WAV\n");
        return -1;
    }

    char subchunk_id[4];
    uint32_t subchunk_size;
    int found_fmt = 0;

    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t byte_rate;
    uint16_t block_align; //block_align = num_channels * bits_per_sample =>  1 * 16 / 8 = 2
    uint16_t bits_per_sample;
    uint32_t sample_rate;

while (fread(subchunk_id, 1, 4, fp) == 4 && fread(&subchunk_size, sizeof(uint32_t), 1, fp) == 1) {
    if (memcmp(subchunk_id, "fmt ", 4) == 0) {
        found_fmt = 1;

        if (subchunk_size < 16) {
            fclose(fp);
            printf("Subchunk size < 16\n");
            return -1;
        }

        if(fread(&audio_format, sizeof(uint16_t), 1, fp) != 1) {
            fclose(fp);
            printf("audio format != 1\n");
            return -1;
        }

        if(fread(&num_channels, sizeof(uint16_t), 1, fp) != 1) {
            fclose(fp);
            printf("channels # != 1\n");
            return -1;
        }

        if(fread(&sample_rate, sizeof(uint32_t), 1, fp) != 1) {
            fclose(fp);
            printf("No sample rate obtained\n");
            return -1;
        }

        if(fread(&byte_rate, sizeof(uint32_t), 1, fp) != 1) {
            fclose(fp);
            printf("No byte rate obtained\n");
            return -1;
        }

        if(fread(&block_align, sizeof(uint16_t), 1, fp) != 1) {
            fclose(fp);
            printf("Block align obtain failure\n");
            return -1;
        }

        if(fread(&bits_per_sample, sizeof(uint16_t), 1, fp) != 1) {
            fclose(fp);
            printf("Bits per sample obtain failed\n");
            return -1;
        }

        if (subchunk_size > 16) {
            long extra = subchunk_size - 16;
            if (extra % 2 != 0) {
            extra++;
            }

            if (fseek(fp, extra, SEEK_CUR) != 0) {
                fclose(fp);
                printf("Skip by extra bytes failed\n");
                return -1;
            }
        }

        if (audio_format != 1 || num_channels != 1 || bits_per_sample != 16) {
            fclose(fp);
            printf("Audio format or channel number or bits per sample invalid\n");
            return -1;
        }

        if (block_align != 2) {
            fclose(fp);
            printf("block align invalid\n");
            return -1;
        }

        if (byte_rate != sample_rate * block_align) {
            fclose(fp);
            printf("byte rate invalid\n");
            return -1;
        }

        break;

    } else {
        long skip_bytes = subchunk_size;
        if (skip_bytes % 2 != 0) {
        skip_bytes++;
        }
        if (fseek(fp, skip_bytes, SEEK_CUR) != 0) {
            fclose(fp);
            return -1;
        }
    }

}

    if (!found_fmt) {
        fclose(fp);
        return -1;
    }

int found_data = 0;
buf->fs = sample_rate;

while (fread(subchunk_id, 1, 4, fp) == 4 && fread(&subchunk_size, sizeof(uint32_t), 1, fp) == 1) {
    if (memcmp(subchunk_id, "data", 4) == 0 ) {
        found_data = 1;
        break;
    } else {
        long skip_bytes = subchunk_size;
        if (skip_bytes %2 != 0) {
            skip_bytes++;
        }
    
        if (fseek(fp, skip_bytes, SEEK_CUR) != 0) {
            fclose(fp);
            return -1;
        }
    
    }
}

    if (!found_data) {
        fclose(fp);
        return -1;
    }

    if (subchunk_size %2 != 0) {
        fclose(fp);
        return -1;
    }

    buf->num_samples = subchunk_size / 2;
    buf->samples = calloc(buf->num_samples, sizeof(double));
    if (buf->samples == NULL) {
        fclose(fp);
        return -1;
    }

    for (unsigned i = 0; i < buf->num_samples; i++) {
        int16_t pcm_sample; //raw audio sample pcm_sample
        
        if (fread(&pcm_sample, sizeof(int16_t), 1, fp) !=1) {
            free(buf->samples);
            buf->samples = NULL;
            buf->num_samples = 0;
            buf->fs = 0;
            fclose(fp);
            return -1;
        }

        buf->samples[i] = pcm_sample / 32768.0;
    }

    fclose(fp);
    return 0;


}

int write_wav_mono(const char *filename, const audio *buf) {
    if (filename == NULL || buf == NULL || buf->samples == NULL || buf->fs == 0 || buf->num_samples == 0) {
        return -1;
    }
    FILE *fp = NULL;
    uint16_t audio_format = 1;
    uint16_t num_channels = 1;
    uint16_t bits_per_sample = 16;
    uint32_t fmt_chunk_size = 16;

    uint16_t block_align = num_channels * bits_per_sample / 8; // (block_align = 2)
    uint32_t byte_rate = buf->fs * block_align;
    uint32_t data_chunk_size = buf->num_samples * block_align;   
    uint32_t riff_chunk_size = 36 + data_chunk_size;
    uint32_t sample_rate = (uint32_t)buf->fs;

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        return -1;
    }

    //header
    if (fwrite("RIFF", 1, 4, fp) != 4) {fclose(fp); return -1;}
    if (fwrite(&riff_chunk_size, sizeof(uint32_t), 1, fp) != 1) {fclose(fp); return -1;}
    if (fwrite("WAVE", 1, 4, fp) != 4) {fclose(fp); return -1;}

    //fmt chunk
    if (fwrite("fmt ", 1, 4, fp) != 4) {fclose(fp); return -1;}
    if (fwrite(&fmt_chunk_size, sizeof(uint32_t), 1, fp) != 1) {fclose(fp); return -1;}
    if (fwrite(&audio_format, sizeof(uint16_t), 1, fp) != 1) {fclose(fp); return -1;}
    if (fwrite(&num_channels, sizeof(uint16_t), 1, fp) != 1) {fclose(fp); return -1;}
    if (fwrite(&sample_rate, sizeof(uint32_t), 1, fp) != 1) {fclose(fp); return -1;}
    if (fwrite(&byte_rate, sizeof(uint32_t), 1, fp) != 1) {fclose(fp); return -1;}
    if (fwrite(&block_align, sizeof(uint16_t), 1, fp) != 1) {fclose(fp); return -1;}
    if (fwrite(&bits_per_sample, sizeof(uint16_t), 1, fp) != 1) {fclose(fp); return -1;}

    //data
    if (fwrite("data", 1, 4, fp) != 4) {fclose(fp); return -1;}
    if (fwrite(&data_chunk_size, sizeof(uint32_t), 1, fp) != 1) { fclose(fp); return -1;}

    for (unsigned i = 0; i < buf->num_samples; i++) {
        double x = buf->samples[i];
        int16_t pcm;

        if (x > 1.0) {x = 1.0;}
        if ( x < -1.0) { x = -1.0;}

        pcm = (int16_t)(x * 32767.0);

        if (fwrite(&pcm, sizeof(int16_t), 1, fp) != 1) {
             fclose(fp); 
             return -1;
        }
    }

    fclose(fp);
    return 0;

}


void free_audio_buffer(audio *buf) {
    if (buf == NULL) {
        return;
    }

    if (buf->samples != NULL) {
        free(buf->samples);
        buf->samples = NULL;
    }

    buf->fs = 0;
    buf->num_samples = 0;
}
