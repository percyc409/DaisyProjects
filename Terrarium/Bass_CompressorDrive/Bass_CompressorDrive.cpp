#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "highshelf.h"
#include <string>
#include "arm_math.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal hw;
dsy_gpio led1;
dsy_gpio led2;

Compressor comp;
Svf filter; // State Variable filter for Freq splitting
Overdrive overdrive;
Highshelf Pre_emphasis, Post_filt; // Pre and Post filter for cleaner waveshaping

Parameter threshold;
Parameter ratio;
Parameter makeup;
Parameter attack;

Parameter driveLevel; // Level of overdrive sig
Parameter drive;

bool bypassComp = true;
bool bypassDrive = true;

enum freqs {high, low};
freqs freq_split;

enum modes {compress, od};
modes ledmode;

void ProcessControls() {

	hw.ProcessAllControls();
	
    //Switches
    ledmode = (hw.switches[Terrarium::SWITCH_2].Pressed()) ? compress : od;
    freq_split = (hw.switches[Terrarium::SWITCH_1].Pressed()) ? high : low;


    //Single-Parameter Knobs
    comp.SetThreshold(threshold.Process());
    comp.SetRatio(ratio.Process());
	comp.SetMakeup(makeup.Process());
    overdrive.SetDrive(drive.Process());
    comp.SetAttack(attack.Process());
    driveLevel.Process();
    
    switch(freq_split) {
        case high:  filter.SetFreq(300.0f);
                    break;
        case low:   filter.SetFreq(150.0f);
                    break;
    }

	//Footswitch
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypassDrive = !bypassDrive;
    }

    if(hw.switches[Terrarium::FOOTSWITCH_2].RisingEdge())
    {
        bypassComp = !bypassComp;
    }
	
    //Led1
	dsy_gpio_write(&led2, !bypassComp);

    //Led2
    float gain;

    switch(ledmode){

        case compress:  // Compression Indicator
                        gain = comp.GetGain();

                        if (gain - makeup.Process() < -0.5f) // Checking whether applied gain  is less than -0.5dB (Without makeup gain)
                            dsy_gpio_write(&led1, true);
                        else 
                            dsy_gpio_write(&led1, false);
                        break;
        
        case od:        dsy_gpio_write(&led1, !bypassDrive);
                        break;
    }

}


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float comp_in, hi_path, comp_out;
	ProcessControls();

    for(size_t i = 0; i < size; i++) {
        
        if(bypassDrive) {
            comp_in = in[0][i];	
            hi_path = 0.0f;	
        }
        else {
			// Parrallel Process Lo and Hi sigs
    		// Hi sigs are passed to overdrive. Lo_sigs are passed to compressor
			filter.Process(in[0][i]);
            hi_path = filter.High();
            comp_in = filter.Low();

            hi_path = Pre_emphasis.Process(hi_path);
            hi_path = overdrive.Process(hi_path) * driveLevel.Value();
			hi_path = Post_filt.Process(hi_path);
        }

		if(bypassComp) {
            out[0][i] = comp_in + hi_path;
            comp.Process(0.0f);
        }
        else {
			// Scales input by 2 and then the output by 0.5
    		// This is because there are 6dB of headroom on the daisy
    		// and currently no way to tell where '0dB' is supposed to be
			comp_out = comp.Process(comp_in * 2.0f) * 0.5f;
            out[0][i] = comp_out + hi_path;     
        }
 
    }
}

void Init(float samplerate)
{
    comp.Init(samplerate);
	comp.AutoMakeup(false);
    comp.SetRelease(0.08f);

    overdrive.Init();

    filter.Init(samplerate);

    Pre_emphasis.Init(samplerate);
	Post_filt.Init(samplerate);
	Pre_emphasis.SetFreq(8000.0f);
	Post_filt.SetFreq(8000.0f);
	Pre_emphasis.SetGain(36.0F);
	Post_filt.SetGain(-36.0F);

    threshold.Init(hw.knob[Terrarium::KNOB_1], -40.0f, 0.0f, Parameter::LINEAR);
    ratio.Init(hw.knob[Terrarium::KNOB_2], 1.0f, 20.0f, Parameter::EXPONENTIAL);
    makeup.Init(hw.knob[Terrarium::KNOB_3], 1.0f, 16.0f, Parameter::LINEAR);
    attack.Init(hw.knob[Terrarium::KNOB_4], 0.005f, 0.05f, Parameter::EXPONENTIAL);

    driveLevel.Init(hw.knob[Terrarium::KNOB_5], 0.00f, 1.0f, Parameter::EXPONENTIAL);
    drive.Init(hw.knob[Terrarium::KNOB_6], 0.0f, 0.8f, Parameter::LINEAR);
} 

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	Init(hw.AudioSampleRate());
	hw.StartAdc();
	hw.StartAudio(AudioCallback);

	led1.pin = hw.seed.GetPin(22);
    led1.mode = DSY_GPIO_MODE_OUTPUT_PP;
    led1.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&led1);

    led2.pin = hw.seed.GetPin(23);
    led2.mode = DSY_GPIO_MODE_OUTPUT_PP;
    led2.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&led2);

	while(1) {
        
    }
}
