#include "daisy_pod.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

const float MAX_FREQ = 35;			//30ms period
const float MIN_FREQ = 0.333;		//3s period

int count, MAX_COUNT, MIN_COUNT;	//count between taps. MAX and MIN value
bool tapping = false;				//True when user is tapping
bool averaging = false;				//True after 2nd tap
float BPS; 							//Audio blocks per second
float freq_tt; 						//Frequency read from tap tempo

Parameter freq_k;   				//Frequency read from knob
float prev_k1 = -1.0f;				//Previous knob position. Used to detect change
bool use_tt = false;				//use tap tempo. Knob values arent read when true

float lfo_out;
Oscillator lfo;

void tap_tempo();					
void UpdateLeds();	
void UpdateKnobs();				

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	tap_tempo();
	UpdateKnobs();
	UpdateLeds();
	
	for (size_t i = 0; i < size; i++)
	{
		out[0][i] = in[0][i];
		out[1][i] = in[1][i];
		lfo_out = lfo.Process();
	}
}

void tap_tempo() {

	if(tapping) {
		count++;

		if(hw.button2.RisingEdge() && count > MIN_COUNT) {

			if (averaging) //3rd plus tap
				freq_tt = 0.6*(freq_tt/2) + 0.4*(BPS/count); // Weighted Averaging
			else { 	
				//2nd tap
				freq_tt = BPS/count; //frequency = BPS/count
				averaging = true;
			}

			use_tt = true;
			lfo.SetFreq(freq_tt);
			count = 0;
		}
		else if(count == MAX_COUNT) {   // After 1/MIN_FREQ seconds no tap, reset values
			count = 0;
			tapping = false;
			averaging = false;
		}
	}
	else if(hw.button2.RisingEdge()) //1st tap
		tapping = true;		
}

void UpdateKnobs()
{
	float new_k1 = hw.knob1.Process();

	if(!use_tt){
		lfo.SetFreq(freq_k.Process());
		prev_k1 = new_k1;
	}
	else if (new_k1 > prev_k1+0.05 || new_k1 < prev_k1-0.05) // When the knob value is changed set use_tt to false
		use_tt = false;
}

void UpdateLeds() {
	if(tapping)	
		hw.led1.Set(0, 0, 1); // blue
	else
		hw.led1.Set(0, 0, 0); // off


	if(lfo_out > 0)
		hw.led2.Set(1, 0, 0); // red
	else
		hw.led2.Set(0, 0, 0); // off

	hw.UpdateLeds();
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	BPS = hw.AudioSampleRate()/hw.AudioBlockSize(); // Blocks per second = sample_rate/block_size
	MAX_COUNT = round(BPS/MIN_FREQ);      			// count = BPS/frequency
	MIN_COUNT = round(BPS/MAX_FREQ);   

	freq_k.Init(hw.knob1, MIN_FREQ, MAX_FREQ, freq_k.LOGARITHMIC); //Mapping knob 1 values to freq_k logarithmically

	//LFO Settings
	lfo.Init(hw.AudioSampleRate());
	lfo.SetWaveform(lfo.WAVE_SQUARE);
	lfo.SetPw(0.25);
	lfo.SetFreq(1.0f);

	hw.StartAdc();
	hw.StartAudio(AudioCallback);
	while(1) {}
}