// Real time Pitch shifter based on NORMALIZED FILTERED CORRELATION TIME-SCALE MODIFICATION
// Paper: https://www.dafx.de/paper-archive/2007/Papers/p007.pdf 

#pragma once
#include "daisysp.h"
#include "EnvelopeFollower.h"
#include <stdlib.h>

using namespace daisysp;

class nfctsm_pitch
{
    public:
        nfctsm_pitch() {}
        ~nfctsm_pitch() {}

        void Init() { 

            for (size_t i = 0; i < BUF_SIZE; i++) {
                audio_buffer[i] = 0.0f; 
                analysis_buffer[i] = 0.0f; 
            }

            wr_ptr = BUF_SIZE / 2; // Middle of the buffer
            rd_ptr =  BUF_SIZE / 2 - static_cast<float>(MIN_DELAY + 0.5 * (MAX_DELAY - MIN_DELAY)); // Delayed to a Mid point between min and max delay from write pointer
            delay = wr_ptr - rd_ptr;
            pitch_ratio = 1.0f;
 
            cf_i = 1;
            crossfading = false;

            lp_filt.Init();
            lp_filt.SetFrequency(0.01875f); // 800Hz
            env_flwr.Setup(48000, 50, 100);
        }

        float Process(float in) {

            // Write to Buffer
            audio_buffer[wr_ptr] = in;

            float lp_in = lp_filt.Process(in);

            env_flwr.Process(lp_in);
            analysis_buffer[wr_ptr] = lp_in * (1.0f/env_flwr.m_env[0]);

            
            wr_ptr = (wr_ptr + 1) % BUF_SIZE;

            // Read from Buffer
            float out;

            if(crossfading) { 

                volatile float cf_a, cf_b, cf_ratio;

                cf_a = LinIntrp(rd_ptr);
                cf_b = LinIntrp(cf_rd_ptr);

                cf_ratio = (static_cast<float>(cf_i )/ static_cast<float>(crossfade_len));

                out = (cf_a * cf_ratio) + (cf_b * (1.0f - cf_ratio));
                cf_i++;

                if (cf_i == crossfade_len) {
                    crossfading = false;
                    cf_i = 1;
                } else {
                    cf_rd_ptr = cf_rd_ptr + pitch_ratio;
                    if (cf_rd_ptr >= static_cast<float>(BUF_SIZE)) cf_rd_ptr = cf_rd_ptr - BUF_SIZE;
                }

            } else {
                out = LinIntrp(rd_ptr);
            }
            
            rd_ptr = rd_ptr + pitch_ratio;
            
            if (rd_ptr >= static_cast<float>(BUF_SIZE)) {
                rd_ptr = rd_ptr - BUF_SIZE;
            }

            // Update and check delay
            delay = wr_ptr - rd_ptr;
            if (delay < 0) delay = delay + BUF_SIZE;

            if (delay >= MAX_DELAY || delay <= MIN_DELAY)  {
                 
                JumpReadPointer(); 
               
            } 
            
            return out;

        }

        void SetPitch(float p) {

            pitch_ratio = (p > 2.0f) ? 2.0f : (p < 0.5f) ? 0.5f : p;

            shift_up = (pitch_ratio > 1.0f) ? true : false;

        }

        void SetSemitones(int s) {

            int st = (s < -12) ? -12 : (s > 12) ? 12 : s;
            
            pitch_ratio = powf(2.0f, (static_cast<float>(st) / 12.0f));

            shift_up = (pitch_ratio > 1.0f) ? true : false;

        } 


        void JumpReadPointer() {

            volatile size_t start_cw;
            volatile  size_t start_sw;

            volatile float corr_out;
            volatile float min_val;
            volatile size_t min_i = 0;
            volatile float sw;      

            start_cw = static_cast<int>(round(rd_ptr));

            if (shift_up) {
                start_sw = (wr_ptr - MAX_T - MIN_DELAY + BUF_SIZE) % BUF_SIZE;
            }
            else {    
                start_sw = (wr_ptr - MAX_T + BUF_SIZE) % BUF_SIZE;
            }

            // Fill correlation windows
            for (size_t i = 0; i < L_CORR_WIN; i++) {
                correlation_window[i] = analysis_buffer[(start_cw + i) % BUF_SIZE];
            }

            //Correlation + find index of minimum
            for (size_t i = 0; i < L_SEARCH_WIN; i = i+25) {
                
                corr_out = 0.0f;

                for (size_t j = 0; j < L_CORR_WIN; j++) {

                    sw = analysis_buffer[(start_sw + i + j) % BUF_SIZE];

                    corr_out += fabs(sw - correlation_window[j]);
                } 

                if (i == 0) {
                    min_val = corr_out;
                } else if (corr_out < min_val) {
                    min_val = corr_out;
                    min_i = i;
                }
            }

            //Update Pointers
            cf_rd_ptr = rd_ptr;  
            rd_ptr = (start_sw + min_i) % BUF_SIZE;     

            crossfading = true;
        }

    protected:


        inline const float LinIntrp(float ptr) const
        {
            int32_t ptr_int   = static_cast<int32_t>(ptr);
            float   ptr_frac  = ptr - static_cast<float>(ptr_int);

            float a = audio_buffer[(ptr_int) % BUF_SIZE];
            float b = audio_buffer[(ptr_int + 1) % BUF_SIZE];
            return a + (b - a) * ptr_frac;
        }


        OnePole lp_filt;
        EnvelopeFollower<1, float> env_flwr;

        const static size_t MAX_DELAY = 1600;
        const static size_t MIN_DELAY = 210;
        const static size_t BUF_SIZE = 2000;
        const size_t MAX_T = 800;  // Period of 60hz in samples
        const size_t crossfade_len = 50;
        const static size_t L_CORR_WIN = 200;
        const size_t L_SEARCH_WIN = 600;

        float audio_buffer[BUF_SIZE];
        float analysis_buffer[BUF_SIZE];
        float correlation_window[L_CORR_WIN];
        int   wr_ptr;
        float rd_ptr;
        float delay;
        float pitch_ratio;
        bool shift_up;

        bool  crossfading;
        float cf_rd_ptr;
        size_t cf_i;

};