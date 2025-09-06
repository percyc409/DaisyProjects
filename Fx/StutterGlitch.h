#pragma once
#include "../Utils/MonoEnvelopeFollower.h"
#include <stdlib.h>

#define MAX_SIZE 24000  // Half a second in samples
#define MIN_SIZE 2400   // 100ms in samples

class StutterGlitch
{
public:

    StutterGlitch() {
        recording = false;
        rd_ptr = 0;
        wr_ptr = 0;
        wait_cnt = 0;
        glitchOn = false;
        slice_len = MIN_SIZE;

        Glitch_Stops = false;
        Mute_Dry = false;
        ThresholdMode = false;
        looplength = MIN_SIZE-100;
        glitch_vol = 0.8f;


        //Init Envelope Follower
	    env_flwr.Setup(48000, 10, 100);

        for (int i = 0; i < MAX_SIZE; i++) {
            buf[i] = 0;
        }
    }

    float Process (float in) {

        float fx_out;

        env_flwr.Process(in);
        
        if (recording) {
            
            rd_ptr = 0;
            wait_cnt = 0;

            buf[wr_ptr] = in;
            wr_ptr++;

            if (wr_ptr == slice_len) {
                recording = false;
                waiting = true;
            }

            fx_out = in;

        } else {

            wr_ptr = 0;
            
            float loop_out = buf[rd_ptr];
            rd_ptr++;
            rd_ptr = rd_ptr % slice_len;


            if (ThresholdMode) {
                thresholdCheck();
            } else {
                randomise();
            }				

            if (Mute_Dry) {
                fx_out = loop_out;
            } else {

                if(glitchOn) {
                    fx_out = in + loop_out * glitch_vol;
                } else {
                    fx_out = in;
                }
                
            }
        }

        return fx_out;
    }

    void inline setLoopLength(float in) { looplength = in; } 

    void inline setRandFreq(float in) { rand_freq = in; } 

    void inline setThreshold(float in) { threshold = in; } 

    void inline setGlitchVol(float in) { glitch_vol = in; }

    void inline setMode(bool in) { ThresholdMode = in; } 

    void inline setRecording (bool in) { recording = in; }

    void inline setGlitchStops (bool in) {Glitch_Stops = in; }

    void inline setMuteDry (bool in) {Mute_Dry = in; }

    bool inline isRecording () { return recording; }

    bool inline getMode () {return ThresholdMode; }

private: 

    int getLoopLength() {

        int len;

        if (looplength < MIN_SIZE) { // When Knob is all the way to the left, LoopLength is randomised
            float rng_out = (float)rand()/RAND_MAX; //Range of 0-1
            len = MIN_SIZE + rng_out*(MAX_SIZE - MIN_SIZE);
        } else {
            len = looplength;
        }

        return len;
    }

    void randomise() { //Randomly Triggers Samplingtask 
	
        if (waiting) {
            wait_cnt++;

            if(wait_cnt > slice_len) {
                waiting = false;
            }

        } else {
            float rng_out = (float)rand()/RAND_MAX; //Range of 0-1
            float rand_f = (Glitch_Stops) ? 2.0f*rand_freq : rand_freq;

            if (rng_out > 1.0f - rand_f) {
                glitchOn = (Glitch_Stops) ? !glitchOn : true; //Randomly Stops the glitch
                recording = true;
                slice_len = getLoopLength(); 
            }
        }

    }

    void thresholdCheck() { //Triggers Sampling When envelop follower detects a loud note

        glitchOn = true;
        
        if (waiting) {
            wait_cnt++;

            if(wait_cnt > slice_len) {
                waiting = false;
            }

        } else if (env_flwr.Out() > threshold) {
            recording = true;
            slice_len = getLoopLength();
        }
        
    }

    MonoEnvelopeFollower<float> env_flwr;

    float buf[MAX_SIZE];

    //Parameters 
    float looplength;
    float threshold;
    float rand_freq;
    float glitch_vol;

    bool ThresholdMode;
    bool Glitch_Stops;
    bool Mute_Dry  = false;

    bool recording = false;
    bool waiting = true;
    bool glitchOn = false;

    int slice_len;
    int wr_ptr, rd_ptr, wait_cnt;
};