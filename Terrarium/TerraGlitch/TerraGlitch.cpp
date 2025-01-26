#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "../../Utils/MonoEnvelopeFollower.h"
#include <stdlib.h>

#define MAX_SIZE 24000  // Half a second in samples
#define MIN_SIZE 4800   // 100ms in samples

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal hw;
dsy_gpio led1;
dsy_gpio led2;

SmoothRandomGenerator rng1, rng2;
MonoEnvelopeFollower<float> env_flwr;

bool bypass = true;

float DSY_SDRAM_BSS buf[MAX_SIZE];
bool recording = false;
bool waiting = true;
bool mute_dry  = false; //TBD add
bool detect=false;
bool ThresholdMode;
int slice_len;
int wr_ptr, rd_ptr, wait_cnt;


void ProcessControls() {
	
	hw.ProcessAllControls();

	//footswitch
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
    }

	//Switches
	ThresholdMode = (hw.switches[Terrarium::SWITCH_1].Pressed()) ? true : false;
	
	dsy_gpio_write(&led1, !bypass);
	dsy_gpio_write(&led2, recording);

}

void randomise() {

	if (waiting) {
		wait_cnt++;

		if(wait_cnt > slice_len * 2) {
			waiting = false;
		}

	} else if (rng1.Process() > 0.999f) {

		recording = true;

		float rng_out = (rng2.Process()+1)/2.0f;  //Range of 0-1
		slice_len = MIN_SIZE + rng_out*(MAX_SIZE - MIN_SIZE); 
	}

}

void thresholdCheck() {

	if (env_flwr.Out() > 0.1) {
		recording = true;

		float rng_out = (rng2.Process()+1)/2.0f;
		slice_len = MIN_SIZE + rng_out*(MAX_SIZE - MIN_SIZE);
	}
	
}


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	ProcessControls();

	for (size_t i = 0; i < size; i++) {
		
		if (bypass) {
			recording = false;
			out[0][i] = in[0][i];
			out[1][i] = in[1][i];
		}
		else {

			float loop_out;

			env_flwr.Process(in[0][i]);
			
			if (recording) {
				
				rd_ptr = 0;
				wait_cnt = 0;

				buf[wr_ptr] = in[0][i];
				wr_ptr++;

				if (wr_ptr == slice_len) {
					recording = false;
					waiting = true;
				}

				out[0][i] = in[0][i];

			} else {

				wr_ptr = 0;
				
				loop_out = buf[rd_ptr];
				rd_ptr++;
				rd_ptr = rd_ptr % slice_len;


				if (ThresholdMode) {
					thresholdCheck();
				} else {
					randomise();
				}				

				if (mute_dry) {
					out[0][i] = loop_out;
				} else {
					out[0][i] = in[0][i] + loop_out;
				}

			}		
		}
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	//Init Random Number Generator
	rng1.Init(hw.AudioSampleRate());
	rng1.SetFreq(hw.AudioSampleRate()*0.9732f);

	rng2.Init(hw.AudioSampleRate());
	rng2.SetFreq(hw.AudioSampleRate()*0.9655f);

	//Init Envelope Follower
	env_flwr.Setup(48000, 10, 100);

	//Init Buffer
	for (int i = 0; i < MAX_SIZE; i++) {
		buf[i] = 0.0f;
	}
	

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
