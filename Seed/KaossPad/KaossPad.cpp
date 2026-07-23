#include "daisy_pad.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyPad hw;

//Effect - Low Pass Filter
LadderFilter lpfilt[2];
Tremolo trem[2];

//Encoder
volatile uint32_t enc_val;
bool enc_button;
bool button1;
bool button2;
bool led_off;

//Midi
uint8_t note;
bool midi_received = false;

//DMA Memory
uint8_t DMA_BUFFER_MEM_SECTION dma_buffer[16];


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();

	if (enc_val==0) {
		lpfilt[0].SetFreq(hw.x_axis*1.15f + 5.f);
		lpfilt[0].SetRes(hw.y_axis/2800.0f);
		lpfilt[1].SetFreq(hw.x_axis*1.15f + 5.f);
		lpfilt[1].SetRes(hw.y_axis/2800.0f);
	} else if (enc_val==1) {
		trem[0].SetFreq(hw.x_axis/100);
		trem[1].SetFreq(hw.x_axis/100);
		trem[0].SetDepth(hw.y_axis/2800.0f);
		trem[1].SetDepth(hw.y_axis/2800.0f);
	}
	

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
	button1 = hw.button1.Pressed();
	button2 = hw.button2.Pressed();

	if (hw.encoder.RisingEdge()) {
		led_off = !led_off;

		if (led_off) {
			hw.SetLedMode(DaisyPad::LED_OFF);
		} else {
			hw.SetLedMode(DaisyPad::XY_TRACKER);
		}
	}

	for (size_t i = 0; i < size; i++)
	{
		
		if (enc_val==0) {
			out[0][i] = lpfilt[0].Process(in[0][i]);
			out[1][i] = lpfilt[1].Process(in[1][i]);
		} else if (enc_val==1) {
			out[0][i] = trem[0].Process(in[0][i]);
			out[1][i] = trem[1].Process(in[1][i]);
		} else {
			out[0][i] = in[0][i];
			out[1][i] = in[1][i];
		}
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
	lpfilt[0].Init(hw.AudioSampleRate());
	lpfilt[0].SetFilterMode(LadderFilter::FilterMode::LP24);
	lpfilt[1].Init(hw.AudioSampleRate());
	lpfilt[1].SetFilterMode(LadderFilter::FilterMode::LP24);
	trem[0].Init(hw.AudioSampleRate());
	trem[1].Init(hw.AudioSampleRate());
	hw.midi.StartReceive();

	note = 0;
	enc_val = 0;
	led_off = false;

	//int last_print = System::GetNow();
	
	while(1) {

		//int midi_type = 0;

		if(hw.midi.HasEvents())
        {
            MidiEvent m = hw.midi.PopEvent();
			midi_received = true;
			//midi_type = m.type;

			if (m.type == NoteOn) {
				note = m.data[0];
				hw.seed.PrintLine("Midi Note: %d", note); 
			}

        }

		hw.ProcessPeripherals();

		//Printing Results
		/*int now = System::GetNow();

		if (now - last_print > 2000) { //Print every 2 seconds

			last_print = now;

			if(!hw.touch_detect.Read()) { 
				hw.seed.PrintLine("X axis: %d", hw.x_axis);
				hw.seed.PrintLine("Y axis: %d", hw.y_axis);
			} else {
				hw.seed.PrintLine("No Touch Detected");
			}

			hw.seed.PrintLine("Encoder Value: %d", enc_val);
			hw.seed.PrintLine("Encoder Button: %d \tButton 1: %d\tButton 2: %d", enc_button, button1, button2);
			hw.seed.PrintLine("Knob 1: %d, Knob 2: %d", static_cast<int>(hw.GetKnobValue(DaisyPad::KNOB_1)*1024), static_cast<int>(hw.GetKnobValue(DaisyPad::KNOB_2)*1024));

			if (midi_received) {
				hw.seed.PrintLine("Midi Message Received!");
				//hw.seed.PrintLine("Midi Message Type: %d", midi_type);
				midi_received = false;
			}
			hw.seed.PrintLine("Midi Note: %d", note); 
			hw.seed.PrintLine("* * * * * * * * * * * * * * * *"); 

		}*/
	}
}
