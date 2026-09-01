#pragma once

/**
Midi Clock Handler Class. Determines Midi Clock Period in Samples and provides synchronised delays and frequency calculations to synchronise effects with external hardware
Midi Clock pulses occur 24 times per quarter note. Thats a rate equal to the BPM * 4 * 24
*/

using namespace daisy;
//using namespace daisysp;

class midi_clock_handler
{
  public:

    enum SyncNoteValue
    {
        SIXTEENTH, 
        TRIPLET,
        EIGHT,
        DOTTED_EIGHT,
        QUARTER,
        TRIPLET_BAR,
        DOTTED_QUARTER,
        HALF,
        BAR,
        TWO_BAR
    };

    midi_clock_handler() {}
    ~midi_clock_handler() {}

    void Init(float samplerate) {
        clock_period = 0;
        last_clock_time = System::GetTick();
        beat_count = 0;
        sample_rate = samplerate;
        tick_freq = System::GetTickFreq();
        ticks_per_sample = tick_freq/sample_rate;
        SetSyncDelay(QUARTER);
    }

    void ClockReceived() {
        uint32_t now = System::GetTick();

        if (now > last_clock_time) { // Handle SysTick overflow by not updating clock period
            clock_period = static_cast<uint32_t>(std::round((now - last_clock_time) / ticks_per_sample));
        }
        
        last_clock_time = now;
        beat_count = (beat_count+1) % 96; // 4 Bars
    }

    inline void StartReceived() {
        beat_count = 0;
    }

    void SetSyncDelay (SyncNoteValue SD) {
        switch (SD) {
            case SIXTEENTH:      sync_multiplier = 6;   break;
            case TRIPLET:        sync_multiplier = 8;   break;
            case EIGHT:          sync_multiplier = 12;  break;
            case DOTTED_EIGHT:   sync_multiplier = 18;  break;
            case QUARTER:        sync_multiplier = 24;  break;
            case TRIPLET_BAR:    sync_multiplier = 32;  break;
            case DOTTED_QUARTER: sync_multiplier = 36;  break;
            case HALF:           sync_multiplier = 48;  break;
            case BAR:            sync_multiplier = 96;  break;
            case TWO_BAR:        sync_multiplier = 192; break;
        }
    }

    inline uint32_t ClockPeriod(){ return clock_period;};
    inline float ClockPeriodSeconds(){ return clock_period/sample_rate;}
    inline float QuarterNotesSeconds(){ return 24.f*clock_period/sample_rate;}
    inline float ClockBPM(){ return  sample_rate/clock_period * 2.5f;} // 60/24 = 2.5f
    inline uint32_t GetSyncDelay(){ return clock_period*sync_multiplier;};
    inline float GetSyncFreq(){ return sample_rate/(clock_period*sync_multiplier);};
    inline uint32_t GetBeatCount(){ return beat_count;};
    inline bool IsQuarterNote(){ return (((beat_count %24) == 1) || ((beat_count %24) == 2));}

  private:
    
    uint32_t clock_period; // Clock Period in Samples
    uint32_t last_clock_time;
    volatile uint32_t beat_count;
    float    sample_rate;
    uint32_t sync_multiplier;
    uint32_t tick_freq;
    float    ticks_per_sample;

};