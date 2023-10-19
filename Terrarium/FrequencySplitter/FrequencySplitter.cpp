#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal hw;
dsy_gpio led1;
dsy_gpio led2;

Svf filter;

Parameter cutoff;

bool bypass = true;
bool lo_sig, hi_sig;

void ProcessControls() {
	
	hw.ProcessAllControls();

	//knobs
	filter.SetFreq(cutoff.Process());

	//switches
	lo_sig = (hw.switches[Terrarium::SWITCH_1].Pressed()) ? true : false;
	hi_sig = (hw.switches[Terrarium::SWITCH_2].Pressed()) ? true : false;

	//footswitch
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
    }
	
	dsy_gpio_write(&led1, !bypass);

}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	ProcessControls();

	for (size_t i = 0; i < size; i++) {
		
		if (bypass) {
			out[0][i] = in[0][i];
		}
		else {
			filter.Process(in[0][i]);

			out[0][i] = 0.0f;

			if(lo_sig)
				out[0][i] = out[0][i] + filter.Low();

			if (hi_sig)
				out[0][i] = out[0][i] + filter.High();
		}
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	filter.Init(hw.AudioSampleRate());

	cutoff.Init(hw.knob[Terrarium::KNOB_1], 100.0f, 400.0f, Parameter::EXPONENTIAL);

	hw.StartAdc();
	hw.StartAudio(AudioCallback);

	//Init LEDs

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
