#pragma once
#ifndef CP_TT
#define CP_TT
#include <stdlib.h>
#include <stdint.h>

template<size_t min_count, size_t max_count>
class Tap_Tempo
{
    public:
        Tap_Tempo() {}
        ~Tap_Tempo() {}

        void Init(float samplerate, size_t blocksize) {
            samplerate_ = samplerate;
            blocksize_ = blocksize;

            tapping_ = false;
            averaging_ = false;
            use_tt_ = false;

            count_ = 0;
            count_avg_ = min_count;

        }

        //Called once every audio block. Increments counter and resets values when max_count time is exceeded
        void Process() {

            if(tapping_)
                count_ = count_ + blocksize_;
                
            if(count_  >= max_count) {   // After max_count samples without a tap, reset values
                count_ = 0;
                tapping_ = false;
                averaging_ = false;
            }
	
        }

        //Called when a tap occurs. Initiates counting and calculates count average
        void Tap() {
           
            if(tapping_) {

                if(count_ > min_count) {
                        
                    if (averaging_) //3rd plus tap
                        count_avg_ = 0.6*(count_avg_) + 0.4*(count_); // Weighted Averaging
                    else { 	
                        //2nd tap
                        count_avg_ = count_;
                        averaging_ = true;
                        use_tt_ = true;
                    }

                    count_ = 0;
                } 
            } else
                tapping_ = true; // 1st tap

        }

        //Returns True while the counting audio blocks between taps
        bool Tapping(){return tapping_;}

        //Use tap tempo. Used to override externally set value with tap tempo value. Must be manually deasserted with Stop_tt()
        bool Use_tt() {return use_tt_;}

        //Deassert tap tempo overide
        void Stop_tt() {use_tt_ = false;}

        //Returns the average tap time in samples 
        size_t Count_Avg() {return count_avg_;}

        //Returns the average tap time in hertz
        float Freq_Avg() {return (float)samplerate_ / (float)count_avg_;}

    private:

        bool tapping_; // True when user is tapping
        bool averaging_; // True after 2nd tap
        bool use_tt_; // Use tap tempo. Used to override externally set value with tap tempo value. Must be manually deasserted with Stop_tt()

        int samplerate_;
        size_t blocksize_;
        size_t count_; // counter to count time in samples between taps
        size_t count_avg_; // average count of time in samples between taps

};

#endif