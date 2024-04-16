#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "shy_fft.h"
#include "STFT_Base.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium; 

#define WINDOW_SIZE 1024
#define LAPS 4

DaisyPetal hw;
dsy_gpio led1;
dsy_gpio led2;

bool bypass = true;

STFT_Base<WINDOW_SIZE, LAPS> FFT;

void ProcessControls() {
	
	hw.ProcessAllControls();

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
			out[1][i] = in[1][i];
		}
		else {	
			FFT.Process(in[0][i], out[0][i]);
		}
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(48); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	
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
