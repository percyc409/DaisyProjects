#include "daisy_pod.h"
#include "daisysp.h"
//#include <cmath>

#define SOFTCLIP 0
#define HARDCLIP 1
#define TANH 2

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

float gain, volume;
int mode = SOFTCLIP;
bool bypass = false;

//Distortion Algortithms
void MySoftClip(float in, float &out);
void MyHardClip(float in, float &out);
void tanhDist(float in, float &out);

//Control Functions
void controls();
void UpdateKnobs();
void UpdateEncoder();
void UpdateLeds();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{

	float outl, outr, inl, inr;

	controls();

	for (size_t i = 0; i < size; i++)
	{
		if(bypass) {
			out[0][i] = in[0][i];
			out[1][i] = in[1][i];

		}
		else {
			inl = in[0][i] * gain;
			inr = in[1][i] * gain;

			switch(mode) {
				case SOFTCLIP:
					MySoftClip(inl, outl);
					MySoftClip(inr, outr);
					break;
				case HARDCLIP:
					MyHardClip(inl, outl);
					MyHardClip(inr, outr);
					break;
				case TANH: 
					tanhDist(inl, outl);
					tanhDist(inr, outr);
					break;
				default:
					outl = outr = 0;
			}

			out[0][i] = outl*volume;
			out[1][i] = outr*volume;
		}
	}
}

void MySoftClip(float in, float &out)      // 1/X DISTORTION FROM FV-1
{
    out = in;
    if ( in > 1.0)
    {
        out = 2.0 - 1.0 / in;
    }
    if ( in < -1.0)
    {
        out = -2.0 - 1.0 / in;
    }
    out = out / 2.0;
}

void MyHardClip(float in, float &out) {   // From DaisyForum
	out = in > 1.f ? 1.f : in;
    out = in < -1.f ? -1.f : in;
}

void tanhDist(float in, float &out) {
	out = tanh(in);
}

void controls() {

	// Knob 1: Gain
	// Knob 2: Volume
	// Encoder: Changes Distortion Algorithm
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
    mode = mode % 3;
}

void UpdateKnobs()
{
	gain = hw.knob1.Process() * 20;
	volume = hw.knob2.Process() * 0.5;
}

void UpdateLeds() {
	if(bypass)
		hw.led1.Set(1, 0, 0); // red
	else
		hw.led1.Set(0, 1, 0); // green


	switch(mode) {
		case SOFTCLIP:
			hw.led2.Set(0, 0, 1);
			break;
		case HARDCLIP:
			hw.led2.Set(1, 0, 0);
			break;
		case TANH:
			hw.led2.Set(0, 1, 1);
			break;
	}

	hw.UpdateLeds();
}


int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartAdc();
	hw.StartAudio(AudioCallback);
	while(1) { }
}
