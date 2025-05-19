#pragma once
#include "../../Utils/shy_fft.h"
#include "../../../ChowDSP/include/math_approx/src/trig_approx.hpp"
#include "STFT_Base_test.h"
#include "stmlib/atan.h"

#define PI 3.1415926535897932384626433832795
#define PI_2 1.5707963267948966192313f
#define TWO_PI 6.28318530718f

template <size_t FFT_SIZE, size_t LAPS>
class PhaseVocoder_Base : public STFT_Base_test<FFT_SIZE, LAPS>
{
public:

    PhaseVocoder_Base() {

        for (size_t n = 0; n < FFT_SIZE/2; n++){
            bin_centre_frequency[n] = (TWO_PI * n * this->stride) / FFT_SIZE;
            last_phase_in[n] = 0.0f;
            last_phase_out[n] = 0.0f;
        }

        phase2dev = (float)FFT_SIZE / ((float)this->stride * TWO_PI);
        dev2phase = (TWO_PI * (float)this->stride) / (float)FFT_SIZE;
        num_bins = FFT_SIZE/2;
    }

    void frequency_process()
    {

        float magnitude;
        float phase;
        float phase_diff;
        float bin_deviation;
        float out_phase;
        size_t newbin;

        //Phase Vocoder Input Processing
        for (size_t i = 0; i < num_bins; i++) {
            
            magnitude = sqrt(this->fftBuff[i + num_bins] * this->fftBuff[i + num_bins] + this->fftBuff[i] * this->fftBuff[i]);
            phase = stmlib::fast_atan2(this->fftBuff[i + num_bins], this->fftBuff[i]) /10000.0f;

            phase_diff = phase - last_phase_in[i];
            phase_diff = wrapPhase(phase_diff - bin_centre_frequency[i]);

            bin_deviation = phase_diff * phase2dev;

            analysis_freq[i] = (float)i - bin_deviation;
            analysis_magnitude[i] = magnitude;
            last_phase_in[i] = phase;

        }

        //Process Pitch Shift
        for (size_t n = 0; n < num_bins; n++) {
            synthesis_freq[n] = synthesis_magnitude[n] = 0.0f;
        }

        for (size_t n = 0; n < num_bins; n++) {

            newbin = static_cast<size_t>(floorf(n * pitch_shift + 0.5f));
            //size_t newbin = static_cast<size_t>(floorf(analysis_freq[n] * pitch_shift + 0.5f);

            if (newbin < num_bins) {
                synthesis_magnitude[newbin] += analysis_magnitude[n];
                synthesis_freq[newbin] = analysis_freq[n] * pitch_shift; 
            }
        }

        //Phase Vocoder Output Processing
        for (size_t i = 0; i < num_bins; i++) {
            
            magnitude = synthesis_magnitude[i];

            bin_deviation = synthesis_freq[i] - (float)i;

            phase_diff = bin_deviation * dev2phase;
            phase_diff += bin_centre_frequency[i];

            out_phase = wrapPhase(last_phase_out[i] + phase_diff);

            this->fftBuff[i]            = magnitude * math_approx::cos_mpi_pi<5,float>(out_phase);
            this->fftBuff[i + num_bins] = magnitude * math_approx::sin_mpi_pi<5,float>(out_phase);

            last_phase_out[i] = out_phase;
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
            return fmodf(phaseIn + PI, TWO_PI) - PI;
        else
            return fmodf(phaseIn - PI, -TWO_PI) + PI;	
    }

    // Polynomial approximating arctangenet on the range -1,1.
    // Max error < 0.005 (or 0.29 degrees)
    float atan_approx(float z)
    {
        const float n1 = 0.97239411f;
        const float n2 = -0.19194795f;
        return (n1 + n2 * z * z) * z;
    }


    float atan2_approx(float y, float x){
        float ay = fabs(y), ax = fabs(x);
        int invert = ay > ax;
        float z = invert ? ax/ay : ay/ax; // [0,1]
        float th = atan_approx(z);        // [0,π/4]
        if(invert) th = PI_2 - th;       // [0,π/2]
        if(x < 0) th = PI - th;          // [0,π]
        th = copysign(th, y);              // [-π,π]
        return th;
    }

    float pitch_shift;
    
    float last_phase_in[FFT_SIZE/2];
    float last_phase_out[FFT_SIZE/2];
    float bin_centre_frequency[FFT_SIZE/2];

    float analysis_freq[FFT_SIZE/2];
    float analysis_magnitude[FFT_SIZE/2];
    float synthesis_freq[FFT_SIZE/2];
    float synthesis_magnitude[FFT_SIZE/2];

    float phase2dev;
    float dev2phase;
    size_t num_bins;

};