#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "PhaseVocoder_Base.h"

#define FFT_SIZE 2048
#define LAPS 8

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal hw;
dsy_gpio led1;
dsy_gpio led2;

bool bypass = true;
bool ramping = false;

Parameter pitch;
Parameter ramp_att;
Parameter ramp_rel;

PhaseVocoder_Base<FFT_SIZE, LAPS> PitchShift;
Adsr ramp;

void ProcessControls() {
	
	hw.ProcessAllControls();

	//footswitch
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
    }

	ramp.SetAttackTime(ramp_att.Process());
	ramp.SetReleaseTime(ramp_rel.Process());

	if (!bypass) {
		PitchShift.set_pitch_shift(pitch.Process());
	} else {
		float ramp_val;

		ramp_val = ramp.Process(hw.switches[Terrarium::FOOTSWITCH_2].Pressed());

		PitchShift.set_pitch_shift(pitch.Process() * ramp_val);
	}

	if (ramp.GetCurrentSegment() == 0) {
		ramping = false;
	} else {
		ramping = true;
	}
	
	dsy_gpio_write(&led1, !bypass);

}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	ProcessControls();

	for (size_t i = 0; i < size; i++) {
		
		if (bypass && !ramping) {
			out[0][i] = in[0][i];
			out[1][i] = in[1][i];
		}
		else {
			//insert effect here
			PitchShift.Process(in[0][i], out[0][i]);
		}
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(96); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_32KHZ);

	pitch.Init(hw.knob[Terrarium::KNOB_1], -1.0f, 1.0f, Parameter::LINEAR);
	ramp_att.Init(hw.knob[Terrarium::KNOB_2], 0.0001f, 0.5f, Parameter::EXPONENTIAL);
	ramp_rel.Init(hw.knob[Terrarium::KNOB_3], 0.0001f, 0.5f, Parameter::EXPONENTIAL);

	ramp.Init(hw.AudioSampleRate());
	ramp.SetDecayTime(0.0f);
	ramp.SetSustainLevel(1.0f);

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
