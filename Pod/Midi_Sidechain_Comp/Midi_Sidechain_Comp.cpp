#include "daisy_pod.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

SyntheticBassDrum CompTrig;
Compressor Comp;

const int midi_channel = 8;

bool ctrl_sel = true; //Selects which parameters to map knobs to
float knob1_prev;
float knob2_prev;
bool lock_p1 = false; // Lock Parameter 1 when ctrl select is toggled
bool lock_p2 = false; // Lock Parameter 2 when ctrl select is toggled

float applied_compression;

void ProcessKnobs() {

	float new_k1 = hw.knob1.Process();
	float new_k2 = hw.knob2.Process();

	if (hw.button2.RisingEdge()) {
		ctrl_sel = !ctrl_sel;
		lock_p1 = true;
		lock_p2 = true;

		knob1_prev = new_k1;
		knob2_prev = new_k2;
	} 

	if (lock_p1) {
		lock_p1 = (new_k1 < knob1_prev - 0.02 || new_k1 > knob1_prev + 0.02) ? false : true;
	} else {

		if (ctrl_sel) {
			Comp.SetThreshold((new_k1 * 60.0f) - 80.0f); // Linear Range : -80 to -20
		} else {
			Comp.SetAttack(new_k1*new_k1*0.099f + 0.001f); // Exponential Range : 0.001 to 0.1
		}

	}

	if (lock_p2) {
		lock_p2 = (new_k2 < knob2_prev - 0.02 || new_k2 > knob2_prev + 0.02) ? false : true;
	} else {

		if (ctrl_sel) {
			Comp.SetRatio(new_k2*new_k2*19.0f + 1.0f); // Exponential Range : 1 to 20 
		} else {
			Comp.SetRelease(new_k2*new_k2*0.149f + 0.001f); // Exponential Range : 0.001 to 0.15
		}

	}

}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	ProcessKnobs();


	if (hw.button1.RisingEdge()) {
		CompTrig.Trig();
	}

	for (size_t i = 0; i < size; i++)
	{
		float sc_source =  CompTrig.Process() * 2;
		Comp.Process(sc_source);

		out[0][i] = Comp.Apply(in[0][i]);
		out[1][i] = Comp.Apply(in[1][i]);
	}

	applied_compression = -Comp.GetGain()/40.f; 
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	CompTrig.Init(hw.AudioSampleRate());
	CompTrig.SetDecay(.4f);
	Comp.Init(hw.AudioSampleRate());
	Comp.AutoMakeup(false);


	hw.StartAdc();
	hw.StartAudio(AudioCallback);
	hw.midi.StartReceive();

	while(1) {

		if(hw.midi.HasEvents())
        {
            MidiEvent m = hw.midi.PopEvent();

			if (m.channel == midi_channel && m.type == NoteOn) {
				CompTrig.Trig();
			}

        }

		if (applied_compression > 1.0f) {
			hw.led1.Set(1.0f, 0, 0); // red
		} else {
			hw.led1.Set(applied_compression, 0, 0); // red
		}
		
		if (ctrl_sel) {
			hw.led2.Set(0.0f, 0.0f, 1.0f);
		} else {
			hw.led2.Set(0.0f, 1.0f, 0.0f);
		}

		hw.UpdateLeds();

		System::Delay(5);

	}
}
