#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "../../Fx/PlaitsReverb/reverb.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal hw;
GPIO led1;
GPIO led2;

Reverb reverb;
uint16_t verb_buffer[16384];
Parameter decay, diffusion, tone, mix, input_gain;

bool bypass = true;

void ProcessControls() {
	
	hw.ProcessAllControls();

	//knobs
	reverb.set_time(decay.Process());
	reverb.set_amount(mix.Process());
	reverb.set_diffusion(diffusion.Process());
	reverb.set_lp(tone.Process());
	reverb.set_input_gain(input_gain.Process());

	//footswitch
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
    }
	
	//Led1
	led1.Write(!bypass);

}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	ProcessControls();

	for (size_t i = 0; i < size; i++) {
		
		if (bypass) {
			out[0][i] = in[0][i];
			out[1][i] = in[1][i];
		}
		else {
			//insert effect here
			float fx_in = in[0][i];
			float dummy_stereo_channel = 0.0f;
			
			reverb.Process(fx_in, dummy_stereo_channel);
			
			out[0][i] = fx_in;
			out[1][i] = 0.0f;
		}
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartAdc();
	hw.StartAudio(AudioCallback);

	//Init LEDs
	GPIO::Config cfg1, cfg2;

    cfg1.mode = GPIO::Mode::OUTPUT;
    cfg1.pull = GPIO::Pull::PULLUP;
    cfg1.speed = GPIO::Speed::LOW;
    cfg1.pin = seed::D22;

	led1.Init(cfg1);

    cfg2.mode = GPIO::Mode::OUTPUT;
    cfg2.pull = GPIO::Pull::PULLUP;
    cfg2.speed = GPIO::Speed::LOW;
    cfg2.pin = seed::D23;

	led2.Init(cfg2);

	reverb.Init(verb_buffer);

	mix.Init(hw.knob[Terrarium::KNOB_1], 0.0f, 1.0f, Parameter::LINEAR);
    decay.Init(hw.knob[Terrarium::KNOB_2], 0.0f, 0.95f, Parameter::LINEAR);
    diffusion.Init(hw.knob[Terrarium::KNOB_3], 0.0f, 0.95f, Parameter::LINEAR);
    tone.Init(hw.knob[Terrarium::KNOB_4], 0.01f, 1.0f, Parameter::LINEAR);
	input_gain.Init(hw.knob[Terrarium::KNOB_5], 0.0f, 1.0f, Parameter::EXPONENTIAL);

	while(1) {}
}
