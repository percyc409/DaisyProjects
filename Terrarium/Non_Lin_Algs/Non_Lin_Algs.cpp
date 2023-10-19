#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "Oversampling.h"
#include "highshelf.h"


using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal hw;
dsy_gpio led1;
dsy_gpio led2;

Oversampling<4,1> Oversampling_o;
Highshelf Pre_emphasis, Post_filt;

Parameter gain;
Parameter volume;

int nl_alg;
bool bypass = true;

void ProcessControls() {
	
	hw.ProcessAllControls();

	//knobs
	gain.Process();
	volume.Process();

	//switches
	nl_alg = (hw.switches[Terrarium::SWITCH_2].Pressed()) ? 1 : 0;
	
	if ((hw.switches[Terrarium::SWITCH_1].Pressed())) 
		nl_alg += 2;

	//footswitches
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
    }

	//leds
	dsy_gpio_write(&led1, !bypass);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//Non-Linear Algorithms
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SoftClip(float in, float &out) {
	out = tanh(in);
}

void HardClip(float in, float &out) { 
	out = in > 1.f ? 1.f : in;
    out = in < -1.f ? -1.f : in;
}

void Fuzz(float in, float &out) {
	out = in >=0 ? in : in/(gain.Value()); 
}

void WaveFold(float in, float &out) {
	out = sin(in);
}

float NonLinearProcess(float in){

	float in_g = in * gain.Value();
	float out = 0.0f;

	switch(nl_alg) {

		case 0: SoftClip(in_g, out);
				break;

		case 1: HardClip(in_g, out);
				break;

		case 2: Fuzz(in_g, out);
				break;

		case 3: WaveFold(in_g, out);
				break;
	}

	return out * volume.Value();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	ProcessControls();

	if (bypass) {
		for (size_t i = 0; i < size; i++) 
			out[0][i] = in[0][i];
	}
	else {

		Oversampling_o.Upsample(in, out, size);
		auto ovrsmp_arr = Oversampling_o.getArray();

		for (int i = 0; i < 8; i++) {

			ovrsmp_arr[0][i] = Pre_emphasis.Process(ovrsmp_arr[0][i]);

			ovrsmp_arr[0][i] = NonLinearProcess(ovrsmp_arr[0][i]);

			ovrsmp_arr[0][i] = Post_filt.Process(ovrsmp_arr[0][i]);

		}

		Oversampling_o.Downsample(in, out, size);

	}	
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	Oversampling_o.Init();
	Pre_emphasis.Init(hw.AudioSampleRate() * 2);
	Post_filt.Init(hw.AudioSampleRate() * 2);
	Pre_emphasis.SetFreq(8000.0f);
	Post_filt.SetFreq(8000.0f);
	Pre_emphasis.SetGain(36.0F);
	Post_filt.SetGain(-36.0F);

	gain.Init(hw.knob[Terrarium::KNOB_1], 1.0f, 30.0f, Parameter::LINEAR);
	volume.Init(hw.knob[Terrarium::KNOB_2], 0.0f, 1.5f, Parameter::EXPONENTIAL);

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
