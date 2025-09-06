#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "arm_math.h"
#include "../../Fx/StutterGlitch.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal hw;
dsy_gpio led1;
dsy_gpio led2;

Compressor comp;
StutterGlitch glitch;
Svf filter; // State Variable filter for Freq splitting

Parameter threshold;
Parameter ratio;
Parameter makeup;
Parameter attack;
Parameter loop_length;
Parameter glitch_param;

bool bypassComp = true;
bool bypassGlitch = true;
//bool bypassDrive = true;

enum modes {compress, od};
modes ledmode;

void ProcessControls() {

	hw.ProcessAllControls();
	
    //Switches
    ledmode = (hw.switches[Terrarium::SWITCH_1].Pressed()) ? compress : od;
    glitch.setMode(hw.switches[Terrarium::SWITCH_2].Pressed());
	glitch.setMuteDry(hw.switches[Terrarium::SWITCH_3].Pressed());

    //Single-Parameter Knobs
    comp.SetThreshold(threshold.Process());
    comp.SetRatio(ratio.Process());
	comp.SetMakeup(makeup.Process());
    comp.SetAttack(attack.Process());
    glitch.setLoopLength(loop_length.Process());

    if (glitch.getMode()) {
		glitch.setThreshold(0.05f + 0.1f * glitch_param.Process());
	} else {
		glitch.setRandFreq( 0.00001f + 0.00004f * glitch_param.Process());
	}
    

	//Footswitch
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypassGlitch = !bypassGlitch;
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
        
        case od:        dsy_gpio_write(&led1, !bypassGlitch);
                        break;
    }

}


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float comp_in, comp_out;
	ProcessControls();

    for(size_t i = 0; i < size; i++) {
        
        comp_in = in[0][i];	

		if(bypassComp) {
            comp_out = comp_in;
            comp.Process(0.0f);
        }
        else {
			// Scales input by 2 and then the output by 0.5
    		// This is because there are 6dB of headroom on the daisy
    		// and currently no way to tell where '0dB' is supposed to be
			comp_out = comp.Process(comp_in * 2.0f) * 0.5f;
        }

        if(bypassGlitch) {
            out[0][i] = comp_out; 
            glitch.setRecording(false);
        } else {
            out[0][i] = glitch.Process(comp_out); 
        }
 
    }
}

void Init(float samplerate)
{
    comp.Init(samplerate);
	comp.AutoMakeup(false);
    comp.SetRelease(0.08f);

    filter.Init(samplerate);

    threshold.Init(hw.knob[Terrarium::KNOB_1], -40.0f, 0.0f, Parameter::LINEAR);
    ratio.Init(hw.knob[Terrarium::KNOB_2], 1.0f, 20.0f, Parameter::EXPONENTIAL);
    makeup.Init(hw.knob[Terrarium::KNOB_3], 1.0f, 16.0f, Parameter::LINEAR);
    attack.Init(hw.knob[Terrarium::KNOB_4], 0.005f, 0.05f, Parameter::EXPONENTIAL);
    loop_length.Init(hw.knob[Terrarium::KNOB_5], MIN_SIZE-100, MAX_SIZE, Parameter::LINEAR);
	glitch_param.Init(hw.knob[Terrarium::KNOB_6], 0.0f , 1.0f, Parameter::LINEAR);

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
