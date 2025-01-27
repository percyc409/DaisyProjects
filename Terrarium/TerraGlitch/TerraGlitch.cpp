#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "../../Utils/MonoEnvelopeFollower.h"
#include <stdlib.h>

#define MAX_SIZE 24000  // Half a second in samples
#define MIN_SIZE 2400   // 100ms in samples

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal hw;
dsy_gpio led1;
dsy_gpio led2;

MonoEnvelopeFollower<float> env_flwr;

Parameter loopLength;
Parameter threshold;
Parameter randFreq;

bool bypass = true;

float DSY_SDRAM_BSS buf[MAX_SIZE];

bool ThresholdMode;
bool Glitch_Stops;
bool Mute_Dry  = false; //TBD add

bool recording = false;
bool waiting = true;
bool glitchOn = false;

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
	Mute_Dry      = (hw.switches[Terrarium::SWITCH_2].Pressed()) ? true : false;
	Glitch_Stops  = (hw.switches[Terrarium::SWITCH_3].Pressed()) ? true : false;
	
	dsy_gpio_write(&led1, !bypass);
	dsy_gpio_write(&led2, recording);

}


int getLoopLength() {

	int len;

	if (loopLength.Process() < MIN_SIZE) { // When Knob is all the way to the left, LoopLength is randomised
		float rng_out = (float)rand()/RAND_MAX; //Range of 0-1
		len = MIN_SIZE + rng_out*(MAX_SIZE - MIN_SIZE);
	} else {
		len = loopLength.Process();
	}

	return len;
}

void randomise() { //Randomly Triggers Samplingtask 
	
	if (waiting) {
		wait_cnt++;

		if(wait_cnt > slice_len) {
			waiting = false;
		}

	} else {
		float rng_out = (float)rand()/RAND_MAX; //Range of 0-1
		float rand_f = (Glitch_Stops) ? 2.0f*randFreq.Process() : randFreq.Process();

		if (rng_out > 1.0f - rand_f) {
			glitchOn = (Glitch_Stops) ? !glitchOn : true; //Randomly Stops the glitch
			recording = true;
			slice_len = getLoopLength(); 
		}
	}

}

void thresholdCheck() { //Triggers Sampling When envelop follower detects a loud note

	if (waiting) {
		wait_cnt++;

		if(wait_cnt > slice_len) {
			waiting = false;
		}

	} else if (env_flwr.Out() > threshold.Process()) {
		recording = true;
		slice_len = getLoopLength();
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

				if (Mute_Dry) {
					out[0][i] = loop_out;
				} else {

					if(glitchOn) {
						out[0][i] = in[0][i] + loop_out;
					} else {
						out[0][i] = in[0][i];
					}
					
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

	//Init Envelope Follower
	env_flwr.Setup(48000, 10, 100);

	//Init Parameters
	loopLength.Init(hw.knob[Terrarium::KNOB_1], MIN_SIZE-100, MAX_SIZE, Parameter::LINEAR);
	threshold.Init(hw.knob[Terrarium::KNOB_2], 0.05f , 0.15f, Parameter::LINEAR);
	randFreq.Init(hw.knob[Terrarium::KNOB_3], 0.00001f , 0.00005f, Parameter::LINEAR);

	//Init Buffer
	for (int i = 0; i < MAX_SIZE; i++) {
		buf[i] = 0.0f;
	}
	slice_len = MIN_SIZE;

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
