#include "daisy_pod.h"
#include "daisysp.h"

#define LP 0
#define HP 1
#define BAND 2
#define NOTCH 3
#define PEAK 4

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

Svf filtL;
Svf filtR;

int mode = LP;
float max_freq;
bool bypass = true;

//Control Functions
void controls();
void UpdateKnobs();
void UpdateEncoder();
void UpdateLeds();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	controls(); 

	for (size_t i = 0; i < size; i++)
	{

		filtL.Process(in[0][i]);
		filtR.Process(in[1][i]);


		switch(mode) {
			case LP:
				out[0][i] = filtL.Low();
				out[1][i] = filtR.Low();
				break;
			case HP:
				out[0][i] = filtL.High();
				out[1][i] = filtR.High();
				break;
			case BAND:
				out[0][i] = filtL.Band();
				out[1][i] = filtR.Band();
				break;
			case NOTCH:
				out[0][i] = filtL.Notch();
				out[1][i] = filtR.Notch();
				break;
			case PEAK:
				out[0][i] = filtL.Peak();
				out[1][i] = filtR.Peak();
				break;
		}

	}
}

void controls() {

	// Knob 1: Frequency Cutoff
	// Knob 2: Resonance
	// Encoder: Changes Filter Type Algorithm
	// Button 2: BypassFX

	hw.ProcessAllControls();
	UpdateKnobs();
	UpdateEncoder();
	
	if(hw.button2.RisingEdge())
		bypass = !bypass;

	UpdateLeds();
	
}

void UpdateEncoder()
{
    mode = mode + abs(hw.encoder.Increment());
    mode = mode % 5;
}

void UpdateKnobs()
{
	float freq = hw.knob1.Process() * max_freq;
	float res = hw.knob2.Process();

	filtL.SetFreq(freq);
	filtR.SetFreq(freq);

	filtL.SetRes(res);
	filtR.SetRes(res);
	
}

void UpdateLeds() {
	if(bypass)
		hw.led1.Set(1, 0, 0); // red
	else
		hw.led1.Set(0, 1, 0); // green


	switch(mode) {
		case LP:
			hw.led2.Set(0, 0, 1); //blue
			break;
		case HP:
			hw.led2.Set(1, 0, 0); //red
			break;
		case BAND:
			hw.led2.Set(0, 1, 0); //green
			break;
		case NOTCH:
			hw.led2.Set(0, 1, 1); //light blue
			break;
		case PEAK:
			hw.led2.Set(1, 0, 1); //purple
			break;
	}

	hw.UpdateLeds();
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	filtL.Init(hw.AudioSampleRate());
	filtR.Init(hw.AudioSampleRate());

	max_freq = hw.AudioSampleRate() / 3;

	hw.StartAdc();
	hw.StartAudio(AudioCallback);
	while(1) {}
}
