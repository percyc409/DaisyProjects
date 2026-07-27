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

    /** LED Matrix Mode */
    enum LedMode
    {
        INIT, 
        CHANGE_STATE,
        XY_TRACKER,
        LED_OFF
    };

    /** 7 Segment Display Mode */
    enum DispMode
    {
        CLEAR, 
        DISP_CODE,
        PARAM1,
        PARAM2,
        XY_POS,
        BPM,
        V_BATT
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

    /** Curves are applied to the processed touchscreen X and Y axis readings */
    enum Curve
    {
        LINEAR,      /**< Linear curve */
        EXPONENTIAL, /**< Exponential curve */
        LOGARITHMIC, /**< Logarithmic curve */
        CUBE,        /**< Cubic curve */
        LAST,        /**< Final enum element. */
    };

    DaisyPad() {}
    ~DaisyPad() {}

    /** Init related stuff. */
    void Init(uint8_t *dma_buff16, bool boost = false);

    /** X and Y axis Parameter Settings */
    void XAxisParam(float min, float max, DaisyPad::Curve curve);
    void YAxisParam(float min, float max, DaisyPad::Curve curve);
    
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
    float ProcessXaxis();
    float ProcessYaxis();

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

    /** Handle Digital Peripherals. Call outside of Audio Callback*/
    void ProcessPeripherals();

    /** Read Touch Screen from TSC2046 via SPI*/
    void ReadTouchscreen();

    /** Update LEDs Display using HT16K33 LED Driver */
    void UpdateLed();

    /* Set the 7 Segment Display Code*/
    inline void Set7SegCode(uint32_t in) {display_code = in;};

    /* Set the 7 Segment Display Mode*/
    inline void SetDispMode(DaisyPad::DispMode mode) {disp_mode = mode;};
    
    /* Get the 7 Segment Display Mode*/
    inline DaisyPad::DispMode GetDispMode() {return disp_mode;}

    /* Set the LED Matrix Mode*/
    void SetLedMode(DaisyPad::LedMode mode);

    /* Get the LED Matrix Mode*/
    inline DaisyPad::LedMode GetLedMode() {return led_mode;}

    /* Get the LED Matrix Mode*/
    inline void SetBPM(float bpm) {bpm_disp_val = bpm;}

    /** Public Members */
    DaisySeed     seed;
    Encoder   encoder;
    AnalogControl knob1,      
        knob2,                
        *knobs[KNOB_LAST];    
    Switch button1,           
        button2,              
        *buttons[BUTTON_LAST];
    GPIO touch_detect;
    MidiUartHandler midi;

  private:

    /** Initialise LED Display */
    void LedMatrixInit();

    /** Human Interface Device Update Rates */
    void SetHidUpdateRates();

    /** Number Digit to 7 seg */
    uint8_t number7seg(int num, bool point=false);

    /** Read Battery Voltage from TSC2046 via SPI */
    void CheckBatteryVoltage();

    //DMA Memory
    uint8_t *led_matrix_buffer;

    SpiHandle touchscreen;
    I2CHandle led_display;
    volatile LedMode led_mode;
    volatile DispMode disp_mode;
    uint32_t display_code;
    int x_axis, y_axis;
    Curve x_curve, y_curve;
    float x_pmin_, x_pmax_, y_pmin_, y_pmax_;
    float x_lmin_, x_lmax_, y_lmin_, y_lmax_;
    float batt_volt;
    float bpm_disp_val;
    uint32_t led_state_count;

};

}

#endif