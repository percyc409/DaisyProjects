#pragma once
#include "shy_fft.h"

/*
The FFT of a length-N signal is always a length-N complex signal.
When the input is real, that complex output signal has some symmetry, so all the information lives in the first N / 2 bins. 
Real FFT functions return those N / 2 complex numbers as an array of N real numbers, but there isn’t a standard convention for how those numbers should be arranged. 
Émilie’s method packs the output of Direct as {real[0], real[1], ..., real[N / 2 - 1], imag[0], imag[1], ..., imag[N / 2 - 1]}. 
The input to Inverse expects the same format. (By comparison, the CMSIS real FFT function packs frequency-domain data like {real[0], imag[0], ..., real[N / 2 - 1], imag[N / 2 - 1]}.)
*/

#define PI 3.1415926535897932384626433832795

template <size_t FFT_SIZE>
class FFT_Handler
{
public:
    FFT_Base()
    {
        fft.Init();
        SAMPLE_BUFFER_SIZE = FFT_SIZE;

        // Calculate the scaling factor (1 / N)
        scalingFactor = 1.0f / static_cast<float>(FFT_SIZE);
        
    }

    void Process(float input, float& output_buffer)
    {
        if (fft_buf_index < SAMPLE_BUFFER_SIZE)
        {
            fftInput[fft_buf_index++] = input;
        }

        if (ifft_buf_index < SAMPLE_BUFFER_SIZE)
        {
            output_buffer = ifftOutput[ifft_buf_index++];
        }
        
        if (fft_buf_index >= SAMPLE_BUFFER_SIZE-1)
        {
            fft_buf_index = 0;
            fft_proc();
        }

        if (ifft_buf_index >= SAMPLE_BUFFER_SIZE)
        {
            ifft_buf_index = 0;
        }
    }

    void fft_proc()
    {
        fft.Direct(fftInput, fftOutput);
        frequency_process();
        fft.Inverse(fftOutput, ifftOutput);
        normalize_ifft(ifftOutput);
    }

    void normalize_ifft(float* input)
    {
        // Apply the scaling factor to each element
        for (size_t i = 0; i < FFT_SIZE; i++)
        {
            input[i] *= scalingFactor;
        }
    }

    virtual void frequency_process()
    {

    
    }

protected:
    float fftOutput[FFT_SIZE];

private:
    ShyFFT<float, FFT_SIZE, RotationPhasor> fft;

    float fftInput[FFT_SIZE];
    float ifftOutput[FFT_SIZE];

    int fft_buf_index = 0;
    int ifft_buf_index = 0;
    int SAMPLE_BUFFER_SIZE = FFT_SIZE;
    float scalingFactor;

};


        //Calculate the Analysis Window
        //for (int n = 0; n < WINDOW_SIZE; n++) {
        //	hann_window[n] = 0.5f * (1.0f -cosf(2.0f * PI * n / (float)(WINDOW_SIZE-1)));
        //}

        //in_buf.init();
	    //out_buf.init();
	    //in_buf.wr_ptr = WINDOW_SIZE/2;


		/*//Write Input to circular Buffer
		for (size_t i = 0; i < size; i++) {
			in_buf.write(in[0][i]);
		}

		//Read FFT inputs from circular buffer, returning rd_ptr to start of next window afterwards
		for (size_t i = 0; i < WINDOW_SIZE; i++) {
			fft_in[i] = in_buf.read() * hann_window[i];
		}

		in_buf.hopback_rd_ptr();

		// FFT
		fft.Direct(fft_in, fft_out);

		// Frequency Domain Processing

		// IFFT
		fft.Inverse(fft_out, ifft_out);

		//Normalise
        for (size_t i = 0; i < WINDOW_SIZE; i++) {
            ifft_out[i] *= scalingFactor;
        }

        //Write IFFT output to overlap and add circular buffer, returning wr_ptr to start of next window afterwards
        for (size_t i = 0; i < WINDOW_SIZE; i++) {
			//out_buf.oa_write(ifft_out[i]);
			out_buf.oa_write(fft_in[i]);
		}

		in_buf.hopback_wr_ptr();

		//Read Output from overlap and add circular buffer
		for (size_t i = 0; i < size; i++) {
			out[0][i] = out_buf.oa_read();
		}*/



        /*
struct circ_buff {

  	float buffer[WINDOW_SIZE];
  	int rd_ptr;
  	int wr_ptr;

  	void init() {
		for(int i = 0; i < WINDOW_SIZE; i++) {
			buffer[i] = 0.0f;
		}

		rd_ptr = 0;
		wr_ptr = 0; 
 	}

	float read () {
		float out = buffer[rd_ptr];
		rd_ptr = (rd_ptr + 1) % WINDOW_SIZE;
		return out;
	}

	float oa_read () {
		float out = buffer[rd_ptr];
		buffer[rd_ptr] = 0.0f;
		rd_ptr = (rd_ptr + 1) % WINDOW_SIZE;
		return out;
	}

	void write (float in) {
		buffer[wr_ptr] = in;
		wr_ptr = (wr_ptr + 1) % WINDOW_SIZE;
	}

	void oa_write (float in) {
		float a = buffer[wr_ptr];
		buffer[wr_ptr] = a + in;
		wr_ptr = (wr_ptr + 1) % WINDOW_SIZE;
	}

	inline void hopback_wr_ptr () { wr_ptr = (wr_ptr - WINDOW_SIZE/2) % WINDOW_SIZE; }
	inline void hopback_rd_ptr () { rd_ptr = (rd_ptr - WINDOW_SIZE/2) % WINDOW_SIZE; }

};

circ_buff in_buf;
circ_buff out_buf;

ShyFFT<float, WINDOW_SIZE, RotationPhasor> fft;

float DSY_SDRAM_BSS fft_in[WINDOW_SIZE];
float DSY_SDRAM_BSS fft_out[2*WINDOW_SIZE];
float DSY_SDRAM_BSS ifft_out[WINDOW_SIZE];
float DSY_SDRAM_BSS hann_window[WINDOW_SIZE];

const float scalingFactor = 1.0f / WINDOW_SIZE; */ 