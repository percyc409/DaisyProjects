#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "nfctsm_ps_bass.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal hw;

nfctsm_ps_bass my_ps;
CrossFade crossfade;
Adsr ramp;

Parameter Mix;
Parameter Shift;
Parameter Attack;

dsy_gpio led1;
dsy_gpio led2;

bool bypass = true;
bool whammying;
int whammy_count = 0;

void ProcessControls() {
	
	hw.ProcessAllControls();

	float shift_semi;

	//Knobs
	crossfade.SetPos(Mix.Process());
	shift_semi = round(Shift.Process());
	ramp.SetAttackTime(Attack.Process());
	ramp.SetReleaseTime(Attack.Process());

	//footswitch
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
    }

	//Pitch Shifting
	float ramp_val = ramp.Process(hw.switches[Terrarium::FOOTSWITCH_2].Pressed());
	float pitch_ratio = powf(2.0f, (shift_semi / 12.0f));

	if (!bypass) {
		my_ps.SetPitch(pitch_ratio - ramp_val*(pitch_ratio-1.0f));
	} else {
		my_ps.SetPitch(1.0f + ramp_val*(pitch_ratio-1.0f));
	}

	whammying = (ramp_val < 0.001) ? false : true; 
		
	dsy_gpio_write(&led1, !bypass);
	dsy_gpio_write(&led2, whammying);

}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	ProcessControls();

	float fx_out;
	float dry;

	for (size_t i = 0; i < size; i++) {
		
		if (bypass && !whammying) {
			out[0][i] = in[0][i];
			out[1][i] = in[1][i];
		}
		else {	
			dry = in[0][i];
			fx_out = my_ps.Process(dry);
			out[0][i] = crossfade.Process(dry, fx_out);
		}
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(48); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	my_ps.Init();

	crossfade.Init(CROSSFADE_CPOW);
	ramp.Init(hw.AudioSampleRate(), hw.AudioBlockSize());
	ramp.SetDecayTime(0.0f);
	ramp.SetSustainLevel(1.0f);

	Mix.Init(hw.knob[Terrarium::KNOB_1], 0.0f, 1.0f, Parameter::LINEAR);
	Shift.Init(hw.knob[Terrarium::KNOB_2], -12.0f, 12.0f, Parameter::LINEAR); //-12 to 12 notes
	Attack.Init(hw.knob[Terrarium::KNOB_3], 0.05f, 1.2f, Parameter::LINEAR); //50 to 1200ms

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
