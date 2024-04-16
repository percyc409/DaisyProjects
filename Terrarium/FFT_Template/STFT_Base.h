#pragma once
#include "shy_fft.h"

/*
FFT_SIZE should be a power of 2

LAPS should be a factor of FFT_SIZE

ShyFFT note:

The FFT of a length-N signal is always a length-N complex signal.
When the input is real, that complex output signal has some symmetry, so all the information lives in the first N / 2 bins. 
Real FFT functions return those N / 2 complex numbers as an array of N real numbers, but there isn’t a standard convention for how those numbers should be arranged. 
Émilie’s method packs the output of Direct as {real[0], real[1], ..., real[N / 2 - 1], imag[0], imag[1], ..., imag[N / 2 - 1]}. 
The input to Inverse expects the same format. (By comparison, the CMSIS real FFT function packs frequency-domain data like {real[0], imag[0], ..., real[N / 2 - 1], imag[N / 2 - 1]}.)
*/

#define PI 3.1415926535897932384626433832795

template <size_t FFT_SIZE, size_t LAPS>
class STFT_Base
{
public:

    STFT_Base()
    {
        fft.Init();

        stride = FFT_SIZE/LAPS;

        // Calculate the scaling factor to normalize read samples (2 / N*LAPS)
        scalingFactor = 2.0f / (static_cast<float>(FFT_SIZE) * LAPS);

        for (size_t n = 0; n < FFT_SIZE; n++) {
        	window[n] = 0.5f * (1.0f -cosf(2.0f * PI * n / (float)(FFT_SIZE-1)));
        }

        for (size_t i = 0; i < LAPS*2; i++) {
            
            write_index[i] = i*-stride; // Initialising the buffers with negative indexs ensures we dont start writing to them until i*-stride samples have passed
            read_index[i]  = 0;
            
            writing[i] = true; // Initially, all buffers are writing buffers. Eventually, there will be LAPS write buffers and LAPS read buffers
            reading[i] = false;
        }
        
    }


    void Process(float input, float& output)
    {
        Write(input);
        output = Read();
    }


    // Applies windowing and writes to all parallel buffers. Number of parallel write buffers = LAPS 
    void Write(float in) {

        for(size_t i = 0; i <LAPS*2; i++){

            if(writing[i]) {


                if(write_index[i] >=0) {
                    buffer[write_index[i]+FFT_SIZE*i] = in * window[write_index[i]];
                }

                write_index[i]++;

                if(write_index[i] == FFT_SIZE) {

                    writing[i] = false;
					reading[i] = true;
					read_index[i] = 0;

                    fft_proc(i);

                }
            }
        }
    }

    // Reads from all parallel buffers and applies windowing and scaling factor. Number of parallel read buffers = LAPS 
    float Read() {

        float out = 0.0f;

        for(size_t i = 0; i <LAPS*2; i++){

            if(reading[i]) {

                out += buffer[read_index[i]+FFT_SIZE*i] * window[read_index[i]];
                
                read_index[i]++;

                if(read_index[i] == FFT_SIZE) {

                    writing[i] = true;
					reading[i] = false;
					write_index[i] = 0;
                }
            }
        } 
        
        out *= scalingFactor;
        return out;
    }

    // Iniates FFT, calls frequency processing function, initiates IFFT
    void fft_proc(size_t buffer_index)
    {
        size_t start = buffer_index*FFT_SIZE;

        fft.Direct((buffer+start), fftBuff);
        frequency_process();
        fft.Inverse(fftBuff, (buffer+start));
    }

    virtual void frequency_process()
    {

    
    }

protected:
    
    float fftBuff[FFT_SIZE];

private:
    
    ShyFFT<float, FFT_SIZE, RotationPhasor> fft;
    size_t stride;
    float scalingFactor;
    float window[FFT_SIZE];

    float buffer[FFT_SIZE*LAPS*2]; // LAPS*2 buffers of size FFT_SIZE in series => {buffer[1], buffer[2], ...., buffer[LAPS*2]}

    int write_index[LAPS*2];
    int read_index[LAPS*2];
    bool writing[LAPS*2];
    bool reading[LAPS*2];

};