# Hybrid-DSP-Audio-Pipeline-in-C
16 bit PCM mono WAV input, supports any sample rate, modular design with plans for reusing this code to integrate into real-time audio projects later.


The main goal of this project is modularity and an examination of filtering and compute tradeoffs between techniques like fir vs iir, radix 2 fft, spectral filtering, etc,. 

User is prompted with input and output destinations for selected WAV file, then prompted if they'd like to select iir or fir filtering. The key for selected filtering was to analyze the computational cost vs filtering quality.

The biggest hurdle in debugging was tuning the Wiener filter. After roughly 200 test runs, I was able to get an output that I was happy with. 

