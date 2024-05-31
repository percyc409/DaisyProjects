#pragma once
#include "shy_fft.h"
#include "STFT_Base.h"

#define PI 3.1415926535897932384626433832795

template <size_t FFT_SIZE, size_t LAPS>
class PhaseVocoder_Base : public STFT_Base<FFT_SIZE, LAPS>
{
public:

    PhaseVocoder_Base() {

        for (size_t n = 0; n < FFT_SIZE/2; n++){
            bin_centre_frequency[n] = 2.0f * PI * n / FFT_SIZE;
            last_phase_in[n] = 0.0f;
            last_phase_out[n] = 0.0f;
        }

    }

    void frequency_process()
    {

        for (size_t i = 0; i < FFT_SIZE/2; i++) {
            
            float magnitude = sqrt(this->fftBuff[i + FFT_SIZE/2] * this->fftBuff[i + FFT_SIZE/2] + this->fftBuff[i] * this->fftBuff[i]);
            float phase = atan2f(this->fftBuff[i + FFT_SIZE/2], this->fftBuff[i]);

            //float magnitude = sqrt(this->fftBuff[i*2 + 1] * this->fftBuff[i*2 + 1] + this->fftBuff[i*2] * this->fftBuff[i*2]);
            //float phase = atan2f(this->fftBuff[i*2 + 1], this->fftBuff[i*2]);

            float phase_diff = phase - last_phase_in[i];
            phase_diff = wrapPhase(phase_diff - bin_centre_frequency[i] * this->stride);

            float bin_deviation = phase_diff * (float)FFT_SIZE / (float)this->stride / (2.0f*PI);

            analysis_freq[i] = (float)i + bin_deviation;
            analysis_magnitude[i] = magnitude;
            last_phase_in[i] = phase;

        }

        process_pitch_shift();


        for (size_t i = 0; i < FFT_SIZE/2; i++) {
            
            float magnitude = synthesis_magnitude[i];

            float bin_deviation = synthesis_freq[i] - i;

            float phase_diff = bin_deviation * 2.0f * PI * (float)this->stride / (float)FFT_SIZE;
            phase_diff += bin_centre_frequency[i] * this->stride;

            float out_phase = wrapPhase(last_phase_out[i] + phase_diff);

            this->fftBuff[i] = magnitude * cosf(out_phase);
            this->fftBuff[i + FFT_SIZE/2] = magnitude * sinf(out_phase);

            //this->fftBuff[i*2] = magnitude * cosf(out_phase);
            //this->fftBuff[i*2 + 1] = magnitude * sinf(out_phase);


            last_phase_out[i] = out_phase;
        }
    
    }

    void process_pitch_shift() {

        for (size_t n = 0; n < FFT_SIZE/2; n++) {
            synthesis_freq[n] = synthesis_magnitude[n] = 0.0f;
        }

        for (size_t n = 0; n < FFT_SIZE/2; n++) {

            size_t newbin = floorf(n * pitch_shift + 0.5);

            if (newbin < FFT_SIZE/2) {
                synthesis_magnitude[newbin] += analysis_magnitude[n];
                synthesis_freq[newbin] = analysis_freq[n] * pitch_shift; 
            }
        }

    }

    //Expects x to be in range -1 to 1 for +/- 1 octave pitch shift range
    void set_pitch_shift(float x) {
        pitch_shift = powf(2.0, x);
    }


private:

    // Wrap the phase to the range -pi to pi
    float wrapPhase(float phaseIn)
    {
        if (phaseIn >= 0)
            return fmodf(phaseIn + PI, 2.0 * PI) - PI;
        else
            return fmodf(phaseIn - PI, -2.0 * PI) + PI;	
    }

    float pitch_shift;
    
    float last_phase_in[FFT_SIZE/2];
    float last_phase_out[FFT_SIZE/2];
    float bin_centre_frequency[FFT_SIZE/2];

    float analysis_freq[FFT_SIZE/2];
    float analysis_magnitude[FFT_SIZE/2];
    float synthesis_freq[FFT_SIZE/2];
    float synthesis_magnitude[FFT_SIZE/2];

};