#include "daisy_pad.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyPad hw;

//Encoder
volatile uint32_t enc_val;
bool enc_button;

//Midi
uint8_t note;
bool midi_received = false;

//DMA Memory
uint8_t DMA_BUFFER_MEM_SECTION dma_buffer[16];


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	if (hw.encoder.Increment() !=0) {
		if ((enc_val == 0) && (hw.encoder.Increment() == -1)) {
		enc_val = 99;
		} else {
			enc_val = (enc_val + hw.encoder.Increment()) % 100;
		}
		hw.SetLedMode(DaisyPad::CHANGE_STATE);
	}
	
	
	hw.display_num = enc_val;
	enc_button = hw.encoder.Pressed();

	for (size_t i = 0; i < size; i++)
	{
		out[0][i] = in[0][i];
		out[1][i] = in[1][i];
	}
}

int main(void)
{
	hw.Init(dma_buffer);
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartAudio(AudioCallback);
	hw.StartAdc();
	hw.seed.StartLog();
	hw.midi.StartReceive();

	note = 0;
	enc_val = 0;

	int last_print = System::GetNow();
	
	while(1) {

		int midi_type = 0;

		if(hw.midi.HasEvents())
        {
            MidiEvent m = hw.midi.PopEvent();
			midi_received = true;
			midi_type = m.type;

			if (m.type == NoteOn) {
				note = m.data[0];
			}

        }

		hw.ProcessPeripherals();
		hw.UpdateLed();

		//Printing Results
		int now = System::GetNow();

		if (now - last_print > 2000) { //Print every 2 seconds

			last_print = now;

			if(!hw.touch_detect.Read()) { 
				hw.seed.PrintLine("X axis: %d", hw.x_axis);
				hw.seed.PrintLine("Y axis: %d", hw.y_axis);
			} else {
				hw.seed.PrintLine("No Touch Detected");
			}

			hw.seed.PrintLine("Encoder Value: %d", enc_val);
			hw.seed.PrintLine("Encoder Button: %d", enc_button);
			hw.seed.PrintLine("Knob 1: %d, Knob 2: %d", static_cast<int>(hw.GetKnobValue(DaisyPad::KNOB_1)*1024), static_cast<int>(hw.GetKnobValue(DaisyPad::KNOB_2)*1024));

			if (midi_received) {
				hw.seed.PrintLine("Midi Message Received!");
				hw.seed.PrintLine("Midi Message Type: %d", midi_type);
				midi_received = false;
			}
			hw.seed.PrintLine("Midi Note: %d", note); 
			hw.seed.PrintLine("* * * * * * * * * * * * * * * *");

		}
	}
}
