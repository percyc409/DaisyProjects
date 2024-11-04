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

daisysp_modified::PitchShifter166 ps;
nfctsm_pitch my_ps;
CrossFade crossfade;

Parameter Mix;
Parameter Shift;
Parameter Attack;

dsy_gpio led1;
dsy_gpio led2;

bool bypass = true;
bool ps_mode = true;
bool jumpAlg;
bool normalise;
bool whammy;
bool whammying;
int whammy_count = 0;

void ProcessControls() {
	
	hw.ProcessAllControls();

	float shift_semi;
	float shift_ratio;


	//footswitch
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
    }

	//Momentary whammy effect
	whammy = hw.switches[Terrarium::FOOTSWITCH_2].Pressed() ? true : false;


	//Knobs
	crossfade.SetPos(Mix.Process());
	shift_semi = round(Shift.Process());
	int whammy_target = round(Attack.Process());

	if (!bypass) {
		ps.SetTransposition(shift_semi);
	} else {

		if(whammy) { //Footswitch2 is pressed
			
			whammying = true;

			if (whammy_count < whammy_target) {
				whammy_count++;
				shift_ratio = (static_cast<float>(whammy_count)/whammy_target)*shift_semi;
			} else {
				shift_ratio = shift_semi;
			}

			ps.SetTransposition(shift_ratio);

		} else {

			if(whammying) { //Footswitch2 released
				whammy_count--;

				if (whammy_count <= 0) {
					whammying = false;
					whammy_count = 0;
				}

				shift_ratio = (static_cast<float>(whammy_count)/whammy_target)*shift_semi;
				ps.SetTransposition(shift_ratio);
			}
		}
	}

	
	//shift_ratio = powf(2.0f, shift_semi/12.0f);

	my_ps.SetSemitones(static_cast<int>(shift_semi));
	

	ps_mode   = hw.switches[Terrarium::SWITCH_1].Pressed() ? true : false;
	jumpAlg   = hw.switches[Terrarium::SWITCH_2].Pressed() ? true : false;
	normalise = hw.switches[Terrarium::SWITCH_3].Pressed() ? true : false;


	my_ps.SetAlg(jumpAlg);
	my_ps.SetNorm(normalise);
	
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

			if (ps_mode) {
				fx_out = ps.Process(dry);


			} else {
				fx_out = my_ps.Process(dry);
			}	

			out[0][i] = crossfade.Process(dry, fx_out);
		}
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(48); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	ps.Init(hw.AudioSampleRate());
	my_ps.Init();

	crossfade.Init(CROSSFADE_CPOW);

	Mix.Init(hw.knob[Terrarium::KNOB_1], 0.0f, 1.0f, Parameter::LINEAR);
	Shift.Init(hw.knob[Terrarium::KNOB_2], -12.0f, 12.0f, Parameter::LINEAR);
	Attack.Init(hw.knob[Terrarium::KNOB_3], 5, MAX_ATTACK, Parameter::LINEAR);

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
