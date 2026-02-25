#pragma once
#include "shy_fft.h"
/*
LOW Memory STFT Base Class. Derived class can define their own Frequency Processing function
This STFT Class optimises for memory at the expense of allocated time for frequency processing
The fft input buffer is read out and windowing needs to be applied in the fft_proc() function before frequency processing takes place and after

FFT_SIZE should be a power of 2

LAPS should be a factor of FFT_SIZE

ShyFFT note:

The FFT of a length-N signal is always a length-N complex signal.
When the input is real, that complex output signal has some symmetry, so all the information lives in the first N / 2 bins. 
Real FFT functions return those N / 2 complex numbers as an array of N real numbers, but there isn’t a standard convention for how those numbers should be arranged. 
Émilie’s method packs the output of Direct as {real[0], real[1], ..., real[N / 2 - 1], imag[0], imag[1], ..., imag[N / 2 - 1]}. 
The input to Inverse expects the same format. (By comparison, the CMSIS real FFT function packs frequency-domain data like {real[0], imag[0], ..., real[N / 2 - 1], imag[N / 2 - 1]}.)
*/

//STFT Class 
#define PI 3.1415926535897932384626433832795

template <size_t FFT_SIZE, size_t LAPS>
class STFT_lmcp
{
    
public:

    STFT_lmcp()
    {
        fft.Init();

        next_buf_end = FFT_SIZE;

        // Calculate the scaling factor to normalize read samples (2 / N*LAPS)
        scalingFactor = 2.0f / (static_cast<float>(FFT_SIZE) * LAPS);

        for (size_t n = 0; n < FFT_SIZE; n++) {
        	window[n] = 0.5f * (1.0f - cosf(2.0f * PI * n / (float)(FFT_SIZE-1)));
        }

        write_index = 0;
        read_index = -FFT_SIZE - STRIDE;
        out_buf_start = 0;
        
    }

    void Reset() 
    {
        next_buf_end = FFT_SIZE;
        write_index = 0;
        read_index = -FFT_SIZE - STRIDE;
        out_buf_start = 0;
    }


    void Process(float input, float& output)
    {
        Write(input);
        output = Read();
    }


    // Applies windowing and writes to all parallel buffers. Number of parallel write buffers = LAPS 
    void Write(float in) {
    
        in_buffer[write_index] = in;    

        write_index = (write_index + 1) % BUF_LEN;

        if(write_index == next_buf_end) {

            next_buf_end = (next_buf_end + STRIDE) % BUF_LEN;
            in_buf_start = (write_index + BUF_LEN - FFT_SIZE) % BUF_LEN; 
            buffer_ready = true;
        }
        
    }

    // Reads from all parallel buffers and applies windowing and scaling factor. Number of parallel read buffers = LAPS 
    float Read() {
        
        float out = 0.0f;

        if(read_index < 0) {
            read_index++;
        } else {

            out = out_buffer[read_index] * scalingFactor;
            
            read_index = (read_index + 1) % BUF_LEN;

        }
        
        return out;
    }

    // Iniates FFT, calls frequency processing function, initiates IFFT
    void fft_proc()
    {
        size_t idx;

        //Fill input buffer
        for (size_t i=0; i < FFT_SIZE; i++) {
            idx = (in_buf_start+i) % BUF_LEN;
            fft_io_buff[i] = in_buffer[idx] * window[i];
        }

        fft.Direct(fft_io_buff, fftBuff);
        frequency_process();
        fft.Inverse(fftBuff, fft_io_buff);

        //Unload output buffer
        for (size_t i=0; i < FFT_SIZE; i++) {

            idx = (out_buf_start+i) % BUF_LEN;

            if(i >= FFT_SIZE-STRIDE) {
                out_buffer[idx] =  fft_io_buff[i] * window[i];
            } else {
                out_buffer[idx] += fft_io_buff[i] * window[i];
            }
        }

        out_buf_start = out_buf_start + STRIDE;

    }

    virtual void frequency_process()
    {

    
    }

	void check_buffer_ready() {
		if (buffer_ready) {
            fft_proc();
            buffer_ready = false;
        }
	}

protected:
    
    static constexpr size_t STRIDE = FFT_SIZE/LAPS;
    float fftBuff[FFT_SIZE];

private:
    
    static constexpr size_t BUF_LEN = FFT_SIZE + STRIDE;

    ShyFFT<float, FFT_SIZE, RotationPhasor> fft;
    float scalingFactor;
    float window[FFT_SIZE];
    float fft_io_buff[FFT_SIZE];

	volatile bool buffer_ready;
    volatile size_t next_buf_end;
    volatile size_t in_buf_start;
    volatile size_t out_buf_start;

    volatile float in_buffer[BUF_LEN]; // LAPS*2 buffers of size FFT_SIZE in series => {buffer[1], buffer[2], ...., buffer[LAPS*2]}
    volatile float out_buffer[BUF_LEN];
    volatile size_t write_index;
    volatile int read_index;

};