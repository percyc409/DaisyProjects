#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "nfctsm_pitch.h"
#include "../../Utils/pitch_shifter166.h"

#define MAX_ATTACK static_cast<size_t>(1000 * 2.0f) // SR/size * 2seconds

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal hw;

nfctsm_pitch my_ps;
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

	//footswitch
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
    }

	//Knobs
	crossfade.SetPos(Mix.Process());
	shift_semi = round(Shift.Process());
	ramp.SetAttackTime(Attack.Process());
	ramp.SetReleaseTime(Attack.Process());

	if (!bypass) {
		my_ps.SetSemitones(static_cast<int>(shift_semi));
		whammying = false;
		ramp.Process(false);
	} else {
		float ramp_val;

		ramp_val = ramp.Process(hw.switches[Terrarium::FOOTSWITCH_2].Pressed());
		float pitch_ratio = powf(2.0f, (shift_semi / 12.0f));

		my_ps.SetPitch(1.0f + ramp_val*(pitch_ratio-1.0f));

		whammying = (ramp_val < 0.001) ? false : true; 
	}
		
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
	Shift.Init(hw.knob[Terrarium::KNOB_2], -12.0f, 12.0f, Parameter::LINEAR);
	Attack.Init(hw.knob[Terrarium::KNOB_3], 0.005f, 0.6f, Parameter::LINEAR);

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
