#include "daisy_pad.h"
#include "daisysp.h"
#include "dig_delay.h"
#include "midi_clock_handler.h"

using namespace daisy;
using namespace daisysp;

#define MAX_DELAY static_cast<size_t>(48000 * 1.0f)
#define MIN_DELAY static_cast<size_t>(48000 * 0.05f)

DaisyPad hw;

/** LED Matrix Mode */
enum FXMode
{
	FILTER, 
	DELAY,
	TREMOLO
};
constexpr int FX_COUNT = 3;

//Effects Classes
Dig_Delay<MAX_DELAY> echo[2];
DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delmem[2];
LadderFilter lpfilt[2];
Tremolo trem[2];
midi_clock_handler midi_clk_detect;

FXMode active_fx = FILTER;
volatile uint32_t param_change_time;
float knob1_prev, knob2_prev;

//Midi
//uint8_t note;
//bool midi_received = false;

//DMA Memory
uint8_t DMA_BUFFER_MEM_SECTION dma_buffer[16];

void ProcessControls() {

	hw.ProcessAllControls();

	if (hw.encoder.Increment() !=0) {
		
		int temp  = static_cast<int>(active_fx) + hw.encoder.Increment();
		active_fx = static_cast<FXMode>((temp + FX_COUNT) % FX_COUNT);

		hw.SetLedMode(DaisyPad::CHANGE_STATE);

		switch (active_fx) {
			
			case FILTER :
				hw.Set7SegCode(0x71387850); // FLTR
				hw.XAxisParam(20.0f, 20000.0f, DaisyPad::EXPONENTIAL);
    			hw.YAxisParam(0.0f, 1.0f, DaisyPad::LINEAR);
				break;
			case DELAY :
				hw.Set7SegCode(0x7939765C); // ECHO
				//hw.XAxisParam(MIN_DELAY, MAX_DELAY, DaisyPad::EXPONENTIAL); // Free Time Mode
				hw.XAxisParam(0, 8, DaisyPad::LINEAR); //Midi Sync Mode
    			hw.YAxisParam(0.0f, 1.0f, DaisyPad::LINEAR);
				break;
			case TREMOLO :
				hw.Set7SegCode(0x78507955); // TREM
				//hw.XAxisParam(0.25f, 20.0f, DaisyPad::LOGARITHMIC); // Free Time Mode
				hw.XAxisParam(0, 8, DaisyPad::LINEAR); //Midi Sync Mode
    			hw.YAxisParam(0.0f, 1.0f, DaisyPad::LINEAR);
				break;
			default : ;

		}

		hw.SetDispMode(DaisyPad::DISP_CODE);
	}

	if (!hw.touch_detect.Read()) {

		switch (active_fx) {
			
			case FILTER :
				lpfilt[0].SetFreq(hw.ProcessXaxis());
				lpfilt[0].SetRes(hw.ProcessYaxis());
				lpfilt[1].SetFreq(hw.ProcessXaxis());
				lpfilt[1].SetRes(hw.ProcessYaxis());
				break;
			case DELAY :
				//echo[0].SetDelayTime(hw.ProcessXaxis()); // Free Time Mode
				//echo[1].SetDelayTime(hw.ProcessXaxis()); // Free Time Mode
				midi_clk_detect.SetSyncDelay(static_cast<midi_clock_handler::SyncNoteValue>(std::floor(hw.ProcessXaxis()))); //Midi Sync Mode
				echo[0].SetDelayTime(midi_clk_detect.GetSyncDelay()); //Midi Sync Mode
				echo[1].SetDelayTime(midi_clk_detect.GetSyncDelay()); //Midi Sync Mode
				echo[0].SetFeedback(hw.ProcessYaxis());
				echo[1].SetFeedback(hw.ProcessYaxis());
				break;
			case TREMOLO :
				//trem[0].SetFreq(hw.ProcessXaxis());
				//trem[1].SetFreq(hw.ProcessXaxis());
				midi_clk_detect.SetSyncDelay(static_cast<midi_clock_handler::SyncNoteValue>(std::floor(hw.ProcessXaxis()))); //Midi Sync Mode
				trem[0].SetFreq(midi_clk_detect.GetSyncFreq());
				trem[1].SetFreq(midi_clk_detect.GetSyncFreq());
				trem[0].SetDepth(hw.ProcessYaxis());
				trem[1].SetDepth(hw.ProcessYaxis());
				break;
			default : ;
		}

		hw.SetDispMode(DaisyPad::XY_POS);
		param_change_time = System::GetNow();
	} 
	
	if (abs(hw.knob1.Value() - knob1_prev) > 0.01f) {
		knob1_prev = hw.knob1.Value();

		switch (active_fx) {
			case FILTER :
				lpfilt[0].SetInputDrive(2*hw.knob1.Value());
				lpfilt[1].SetInputDrive(2*hw.knob1.Value());
				break;
			case DELAY :
				break;
			case TREMOLO :
				break;
			default : ;
		}

		hw.SetDispMode(DaisyPad::PARAM1);
		param_change_time = System::GetNow();
	}
	if (abs(hw.knob2.Value() - knob2_prev) > 0.01f) {
		knob2_prev = hw.knob2.Value();

		switch (active_fx) {
			case FILTER :
				break;
			case DELAY :
				break;
			case TREMOLO :
				break;
			default : ;
		}

		hw.SetDispMode(DaisyPad::PARAM2);
		param_change_time = System::GetNow();
	}


	if (hw.encoder.RisingEdge()) {
		
		if(hw.button1.Pressed()) {
			hw.SetDispMode(DaisyPad::BPM);
		} else {
			hw.SetDispMode(DaisyPad::V_BATT);
		}
		param_change_time = System::GetNow();
	}
}


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	ProcessControls();

	for (size_t i = 0; i < size; i++)
	{
		switch (active_fx) {
			
			case FILTER :
				if (!hw.touch_detect.Read()) {
					out[0][i] = lpfilt[0].Process(in[0][i]);
					out[1][i] = lpfilt[1].Process(in[1][i]);
				} else {
					out[0][i] = in[0][i];
					out[1][i] = in[1][i];
				}
				break;
			case DELAY :
				out[0][i] = in[0][i] + 0.7f*echo[0].Process(in[0][i]);
				out[1][i] = in[1][i] + 0.7f*echo[1].Process(in[1][i]);
				break;
			case TREMOLO :
				out[0][i] = trem[0].Process(in[0][i]);
				out[1][i] = trem[1].Process(in[1][i]);
				break;
			default :
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
	hw.Set7SegCode(0x71387850); // FLTR
	hw.XAxisParam(20.0f, 20000.0f, DaisyPad::EXPONENTIAL);
    hw.YAxisParam(0.0f, 1.0f, DaisyPad::LINEAR);
	
	lpfilt[0].Init(hw.AudioSampleRate());
	lpfilt[0].SetFilterMode(LadderFilter::FilterMode::BP24);
	lpfilt[1].Init(hw.AudioSampleRate());
	lpfilt[1].SetFilterMode(LadderFilter::FilterMode::BP24);
	delmem[0].Init();
	delmem[1].Init();
	echo[0].Init(&delmem[0]);
	echo[1].Init(&delmem[1]);
	trem[0].Init(hw.AudioSampleRate());
	trem[1].Init(hw.AudioSampleRate());
	midi_clk_detect.Init(hw.AudioSampleRate());
	hw.midi.StartReceive();

	hw.StartAudio(AudioCallback);
	hw.StartAdc();
	
	while(1) {

		hw.ProcessPeripherals();

		if(hw.midi.HasEvents())
        {
            MidiEvent m = hw.midi.PopEvent();

			if (m.type == SystemRealTime && m.srt_type == TimingClock) {
				midi_clk_detect.ClockReceived();
				hw.SetBPM(midi_clk_detect.ClockBPM());
			}

        }

		//Return Display to Display Code after 2 Seconds
		if ((hw.GetDispMode() != DaisyPad::DISP_CODE) && (hw.GetDispMode() != DaisyPad::CLEAR)) {
			if (System::GetNow() - param_change_time > 2000) {
				hw.SetDispMode(DaisyPad::DISP_CODE);
			}
		}
	}
}
