#pragma once
#ifndef DSY_PAD
#define DSY_PAD

#include "../../../DaisyExamples/libDaisy/src/daisy_seed.h"
#include "cts_encoder.h"

namespace daisy
{
/**
    @brief Class that handles initializing all of the hardware specific to the Daisy Pad. \n 
    Helper funtions are also in place to provide easy access to built-in controls and peripherals.
    @author Conall Percy
    @date March 2026
    @ingroup boards
*/

class DaisyPad
{
  public:

    /** LedMode */
    enum LedMode
    {
        INIT, 
        CHANGE_STATE,
        XY_TRACKER,
        LED_OFF
    };

    /** Switches */
    enum Sw
    {
        BUTTON_1,   
        BUTTON_2,   
        BUTTON_LAST,
    };

    /** Knobs */
    enum Knob
    {
        KNOB_1,  
        KNOB_2,  
        KNOB_LAST
    };

    DaisyPad() {}
    ~DaisyPad() {}

    /** Init related stuff. */
    void Init(uint8_t *dma_buff16, bool boost = false);

    /** Starts the callback
    \param cb Interleaved callback function
    */
    void StartAudio(AudioHandle::InterleavingAudioCallback cb);

    /** Starts the callback
    \param cb multichannel callback function
    */
    void StartAudio(AudioHandle::AudioCallback cb);

    /**
       Switch callback functions
       \param cb New interleaved callback function.
    */
    void ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb);

    /**
       Switch callback functions
       \param cb New multichannel callback function.
    */
    void ChangeAudioCallback(AudioHandle::AudioCallback cb);

    /** Stops the audio if it is running */
    void StopAudio();

    /** Updates the Audio Sample Rate, and reinitializes.
     ** Audio must be stopped for this to work.
     */
    void SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate);

    /** Returns the audio sample rate in Hz as a floating point number */
    float AudioSampleRate();

    /** Sets the number of samples processed per channel by the audio callback */
    void SetAudioBlockSize(size_t blocksize);

    /** Returns the number of samples per channel in a block of audio */
    size_t AudioBlockSize();

    /** Returns the rate in Hz that the Audio callback is called */
    float AudioCallbackRate();

    /** Start analog to digital conversion */
    void StartAdc();

    /** Stops Transfering data from the ADC */
    void StopAdc();

    /* Returns the most recent ADC reading from the knob */
    float GetKnobValue(Knob k);

    /** Call at same rate as analog reads for smooth reading */
    void ProcessAnalogControls();

    /** Process digital controls */
    void ProcessDigitalControls();

    /** Process Analog and Digital Controls */
    inline void ProcessAllControls()
    {
        ProcessAnalogControls();
        ProcessDigitalControls();
    }

    /** Handle Digital Peripherals. Keep outside of Audio Callback*/
    void ProcessPeripherals();

    /** Initialise LED Display */
    void LedMatrixInit();

    /** Update LEDs Display using HT16K33 LED Driver */
    void UpdateLed();

    /* Set the LED Matrix Mode*/
    void SetLedMode(DaisyPad::LedMode mode);

    /** Public Members */
    DaisySeed     seed;
    Cts_Encoder   encoder;
    AnalogControl knob1,      
        knob2,                
        *knobs[KNOB_LAST];    
    Switch button1,           
        button2,              
        *buttons[BUTTON_LAST];
    GPIO touch_detect;
    MidiUartHandler midi;
    int x_axis, y_axis;
    int display_num;

  private:
    void SetHidUpdateRates();
    uint8_t number7seg(int num);

    //DMA Memory
    uint8_t *led_matrix_buffer;

    SpiHandle touchscreen;
    I2CHandle led_display;
    volatile LedMode led_mode;
    uint32_t last_screen_read;
    uint32_t last_led_update;
    uint32_t led_state_count;

};

}

#endif