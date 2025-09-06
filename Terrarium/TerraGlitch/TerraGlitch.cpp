#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "../../Fx/StutterGlitch.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal hw;
dsy_gpio led1;
dsy_gpio led2;

StutterGlitch glitch;

Parameter loopLength;
Parameter param;
Parameter glitch_vol;

bool bypass = true;


void ProcessControls() {
	
	hw.ProcessAllControls();

	//footswitch
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
    }

	//Switches
	glitch.setMode(hw.switches[Terrarium::SWITCH_1].Pressed());
	glitch.setMuteDry(hw.switches[Terrarium::SWITCH_2].Pressed());
	glitch.setGlitchStops(hw.switches[Terrarium::SWITCH_3].Pressed());

	//Knobs
	glitch.setLoopLength(loopLength.Process());
	glitch.setGlitchVol(glitch_vol.Process());

	if (glitch.getMode()) {
		glitch.setThreshold(0.05f + 0.1f * param.Process());
	} else {
		glitch.setRandFreq( 0.00001f + 0.00004f * param.Process());
	}
	
	dsy_gpio_write(&led1, !bypass);
	dsy_gpio_write(&led2, glitch.isRecording());

}


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	ProcessControls();

	for (size_t i = 0; i < size; i++) {
		
		if (bypass) {
			glitch.setRecording(false);
			out[0][i] = in[0][i];
			out[1][i] = 0.0f;
		}
		else {

			out[0][i] = glitch.Process(in[0][i]);
			out[1][i] = 0.0f;

		}		
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	//Init Parameters
	loopLength.Init(hw.knob[Terrarium::KNOB_1], MIN_SIZE-100, MAX_SIZE, Parameter::LINEAR);
	param.Init(hw.knob[Terrarium::KNOB_2], 0.0f , 1.0f, Parameter::LINEAR);
	glitch_vol.Init(hw.knob[Terrarium::KNOB_3], 0.0f , 1.0f, Parameter::EXPONENTIAL);
	//threshold.Init(hw.knob[Terrarium::KNOB_2], 0.05f , 0.15f, Parameter::LINEAR);
	//randFreq.Init(hw.knob[Terrarium::KNOB_3], 0.00001f , 0.00005f, Parameter::LINEAR);

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
