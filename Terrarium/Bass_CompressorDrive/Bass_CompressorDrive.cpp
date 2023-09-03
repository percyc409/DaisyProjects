#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include <string>

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal hw;
dsy_gpio led1;
dsy_gpio led2;

Compressor comp;

Parameter attack;
Parameter release;
Parameter ratio;
Parameter threshold;
Parameter makeup;

bool bypass = true;
bool autogain = false;
bool last_autogain = false;

void ProcessControls() {

	hw.ProcessAllControls();
	//controls
    comp.SetThreshold(threshold.Process());
    comp.SetRatio(ratio.Process());
    comp.SetAttack(attack.Process());
    comp.SetRelease(release.Process());
	comp.SetMakeup(makeup.Process());

	//footswitch
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
    }
	
	dsy_gpio_write(&led1, !bypass);

	//Compression Indicator
	float gain = comp.GetGain();

	if (gain - makeup.Process() < -0.5f) //Checking whether applied gain  is less than -0.5dB (Without makeup gain)
		dsy_gpio_write(&led2, true);
	else 
		dsy_gpio_write(&led2, false);

}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float sig, dry_in;
	ProcessControls();

    for(size_t i = 0; i < size; i++) {
        
		if(bypass) {
            out[0][i] = in[0][i];	
        }
        else {
			// Scales input by 2 and then the output by 0.5
    		// This is because there are 6dB of headroom on the daisy
    		// and currently no way to tell where '0dB' is supposed to be
			dry_in = in[0][i] * 2.0f;
			sig = comp.Process(dry_in);  
            out[0][i]  = sig * 0.5f;        
        }
    }
}

void Init(float samplerate)
{
    comp.Init(samplerate);
	comp.AutoMakeup(false);

    attack.Init(hw.knob[Terrarium::KNOB_4], 0.01f, 1.0f, Parameter::EXPONENTIAL);
    release.Init(hw.knob[Terrarium::KNOB_5], 0.01f, 1.0f, Parameter::EXPONENTIAL);
    ratio.Init(hw.knob[Terrarium::KNOB_2], 1.0f, 40.0f, Parameter::EXPONENTIAL);
    threshold.Init(hw.knob[Terrarium::KNOB_1], -80.0f, 0.0f, Parameter::LINEAR);
    makeup.Init(hw.knob[Terrarium::KNOB_3], 1.0f, 40.0f, Parameter::LINEAR);  // log?
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

	while(1) {}
}
