#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "nfctsm_pitch.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal hw;
PitchShifter ps;
nfctsm_pitch my_ps;

Parameter DryVol;
Parameter WetVol;
Parameter Shift;

dsy_gpio led1;
dsy_gpio led2;

bool bypass = true;
bool ps_mode = true;
bool filterOn;
bool jumpAlg;
bool normalise;
bool use_hp;

OnePole delay_lp;

void ProcessControls() {
	
	hw.ProcessAllControls();

	float shift_semi;
	float shift_ratio;

	shift_semi = round(Shift.Process());
	shift_ratio = powf(2.0f, shift_semi/12.0f);

	ps.SetTransposition(shift_semi);
	//my_ps.SetPitch(shift_ratio);
	my_ps.SetSemitones(static_cast<int>(shift_semi));

	//footswitch


    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
    }

	if(hw.switches[Terrarium::FOOTSWITCH_2].RisingEdge())
    {
        ps_mode = !ps_mode;
    }

	filterOn  = hw.switches[Terrarium::SWITCH_1].Pressed() ? true : false;
	jumpAlg   = hw.switches[Terrarium::SWITCH_2].Pressed() ? true : false;
	normalise = hw.switches[Terrarium::SWITCH_3].Pressed() ? true : false;
	use_hp    = hw.switches[Terrarium::SWITCH_4].Pressed() ? true : false;

	my_ps.SetAlg(jumpAlg);
	my_ps.UseHp(use_hp);
	my_ps.SetNorm(normalise);
	
	dsy_gpio_write(&led1, !bypass);
	dsy_gpio_write(&led2, ps_mode);

}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	ProcessControls();

	float fx_out;
	float dry;

	for (size_t i = 0; i < size; i++) {
		
		if (bypass) {
			out[0][i] = in[0][i];
			out[1][i] = in[1][i];
		}
		else {
					
			dry = in[0][i];

			if (ps_mode) {
				fx_out = my_ps.Process(dry);
				
				if(filterOn) {
					fx_out = delay_lp.Process(fx_out);
				}

			} else {
				fx_out = ps.Process(dry);

			    if(filterOn) {
					fx_out = delay_lp.Process(fx_out);
				}
			}	

			out[0][i] = fx_out * WetVol.Process() + dry * DryVol.Process();
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
	delay_lp.Init();
	delay_lp.SetFrequency(0.0625f);

	DryVol.Init(hw.knob[Terrarium::KNOB_1], 0.0f, 1.0f, Parameter::EXPONENTIAL);
	WetVol.Init(hw.knob[Terrarium::KNOB_2], 0.0f, 1.0f, Parameter::EXPONENTIAL);
	Shift.Init(hw.knob[Terrarium::KNOB_3], -12.0f, 12.0f, Parameter::LINEAR);

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
