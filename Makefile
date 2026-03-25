CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -g -std=c11
LDFLAGS = -lm

SRC = main.c \
      audio_io.c \
      filters.c \
      framing.c \
      fft.c \
      overlap_add.c \
      spectral.c \
      window.c

OBJ = $(SRC:.c=.o)
OUT = dsp_app

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(OUT)