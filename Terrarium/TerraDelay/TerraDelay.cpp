#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "taptempo.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

#define MAX_DELAY static_cast<size_t>(48000 * 3.5f)
#define MIN_DELAY static_cast<size_t>(48000 * 0.05f)

DaisyPetal hw;
dsy_gpio led1;
dsy_gpio led2;

DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delmem;
Tap_Tempo<MIN_DELAY, MAX_DELAY> time_tt;

struct delay
{
    DelayLine<float, MAX_DELAY> *del;
    float currentDelay;
    float delayTarget;
    float feedback;

    float Process(float in)
    {
        //set delay times
        fonepole(currentDelay, delayTarget, .0002f);
        del->SetDelay(currentDelay);

        float read = del->Read();
        del->Write((feedback * read) + in);

        return read;
    }
};

delay delay_fx;

Parameter time;
Parameter feedback;
Parameter mix;

int time_old;
bool bypass = true;

void ProcessControls() {
	
	hw.ProcessAllControls();
	time_tt.Process();

	//knobs
	delay_fx.feedback = feedback.Process();
	mix.Process();

	if(time_tt.Use_tt()) {
		delay_fx.delayTarget = time_tt.Count_Avg();

		if (abs (time_old - time.Process()) > 8000)
			time_tt.Stop_tt();

	} else
		delay_fx.delayTarget = time.Process();

	//footswitch
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
    }
	
	if(hw.switches[Terrarium::FOOTSWITCH_2].RisingEdge())
    {
        time_tt.Tap();
		time_old = time.Value();
    }
	
	//LEDs
	dsy_gpio_write(&led1, !bypass);
	dsy_gpio_write(&led2, time_tt.Tapping());

}


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	ProcessControls();

	float del_out;

	for (size_t i = 0; i < size; i++) {
		
		if (bypass) {
			out[0][i] = in[0][i];
		}
		else {
			
			del_out = delay_fx.Process(in[0][i]);

			out[0][i] = in[0][i] + mix.Value() * del_out;
		}
	}
}

void Init() {

	delmem.Init();
	delay_fx.del = &delmem;

	time.Init(hw.knob[Terrarium::KNOB_1], MIN_DELAY, MAX_DELAY, Parameter::LOGARITHMIC);
	feedback.Init(hw.knob[Terrarium::KNOB_2], 0.0f, 1.0f, Parameter::LINEAR);
	mix.Init(hw.knob[Terrarium::KNOB_3], 0.0f, 1.0f, Parameter::EXPONENTIAL);

	time_tt.Init(hw.AudioSampleRate(), hw.AudioBlockSize());

}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	Init();

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
