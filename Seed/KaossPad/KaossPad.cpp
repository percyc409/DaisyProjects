#include "daisy_pad.h"
#include "daisysp.h"
#include "dig_delay.h"
#include "midi_clock_handler.h"

using namespace daisy;
using namespace daisysp;

#define MAX_DELAY static_cast<size_t>(48000 * 0.6f)
#define MIN_DELAY static_cast<size_t>(48000 * 0.1f)

DaisyPad hw;

/** Multi Fx Effects Mode */
enum FXMode
{
	BPF, 
	DELAY,
	DECIMATOR,
	HPF_DELAY,
	TREMOLO
};
constexpr int FX_COUNT = 5;

//Effects Objects and Associated Variables
Dig_Delay<MAX_DELAY> echo[2];
DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delmem[2];
bool ClkSync = false;

LadderFilter filter[2];
bool type24db = false; //24db or 12db roll off enable
bool SCCompressorEnable = false;

Decimator LoFi[2];
bool SmoothCrush = false;

Tremolo trem[2];

//Support Object and Parameters
midi_clock_handler midi_clk_detect;
FXMode active_fx = BPF;
volatile uint32_t param_change_time;
float knob1_prev, knob2_prev;
Parameter knob1_exp;

//Global Scope DMA Memory Buffer required for Daisy Pad Initialisation
uint8_t DMA_BUFFER_MEM_SECTION dma_buffer[16];

void ProcessControls() {

	hw.ProcessAllControls();

	// - - - - Rotary Encoder - - - - 
	if (hw.encoder.Increment() !=0) {
		
		int temp  = static_cast<int>(active_fx) + hw.encoder.Increment();
		active_fx = static_cast<FXMode>((temp + FX_COUNT) % FX_COUNT);

		hw.SetLedMode(DaisyPad::CHANGE_STATE);

		switch (active_fx) {
			
			case BPF :
				hw.Set7SegCode(0x71387850); // FLTR
				hw.XAxisParam(100.0f, 14000.0f, DaisyPad::EXPONENTIAL);
    			hw.YAxisParam(0.0f, 1.0f, DaisyPad::LINEAR);
				if (type24db) {
					filter[0].SetFilterMode(LadderFilter::FilterMode::BP24);
					filter[1].SetFilterMode(LadderFilter::FilterMode::BP24);
				} else {
					filter[0].SetFilterMode(LadderFilter::FilterMode::BP12);
					filter[1].SetFilterMode(LadderFilter::FilterMode::BP12);
				}
				break;
			case DELAY :
				hw.Set7SegCode(0x7939765C); // ECHO
				if(ClkSync) {
					hw.XAxisParam(0, 8, DaisyPad::LINEAR); //Midi Sync Mode
				} else {
					hw.XAxisParam(MIN_DELAY, MAX_DELAY, DaisyPad::LINEAR); // Free Time Mode
				}
				hw.YAxisParam(0.0f, 1.2f, DaisyPad::LINEAR);
				echo[0].SetMix(1.0f);
				echo[1].SetMix(1.0f);
				echo[0].SetTone(0.6f);
				echo[1].SetTone(0.6f);
				break;
			case DECIMATOR :
				hw.Set7SegCode(0x385C7130); // LoFi
				hw.XAxisParam(0.0f, 1.0f, DaisyPad::LINEAR);
				hw.YAxisParam(0.0f, 1.0f, DaisyPad::LINEAR);
				break;
			case HPF_DELAY :
				hw.Set7SegCode(0x76F35E30); // HP.DL
				hw.XAxisParam(100.0f, 8000.0f, DaisyPad::LINEAR);
				if(ClkSync) {
					hw.YAxisParam(0, 8, DaisyPad::LINEAR); //Midi Sync Mode
				} else {
					hw.YAxisParam(MIN_DELAY, MAX_DELAY, DaisyPad::LINEAR); // Free Time Mode
				}
				filter[0].SetFilterMode(LadderFilter::FilterMode::HP12);
				filter[1].SetFilterMode(LadderFilter::FilterMode::HP12);
				filter[0].SetRes(0.7f);
				filter[1].SetRes(0.7f);
				echo[0].SetFeedback(0.9f);
				echo[1].SetFeedback(0.9f);
				echo[0].SetMix(1.0f);
				echo[1].SetMix(1.0f);
				echo[0].SetTone(1.0f);
				echo[1].SetTone(1.0f);

				break;
			case TREMOLO :
				hw.Set7SegCode(0x78507955); // TREM
				if (ClkSync) {
					hw.XAxisParam(0, 8, DaisyPad::LINEAR); //Midi Sync Mode
				} else {
					hw.XAxisParam(0.25f, 20.0f, DaisyPad::EXPONENTIAL); // Free Time Mode
				}
    			hw.YAxisParam(0.0f, 1.0f, DaisyPad::LINEAR);
				break;
			default : ;

		}

		hw.SetDispMode(DaisyPad::DISP_CODE);
	}

	// - - - - Touch Screen - - - - 
	if (!hw.touch_detect.Read()) {

		switch (active_fx) {
			
			case BPF :
				filter[0].SetFreq(hw.ProcessXaxis());
				filter[0].SetRes(hw.ProcessYaxis());
				filter[1].SetFreq(hw.ProcessXaxis());
				filter[1].SetRes(hw.ProcessYaxis());
				break;
			case DELAY :
				if (ClkSync) {
					midi_clk_detect.SetSyncDelay(static_cast<midi_clock_handler::SyncNoteValue>(std::floor(hw.ProcessXaxis()))); //Midi Sync Mode
					echo[0].SetDelayTime(midi_clk_detect.GetSyncDelay()); //Midi Sync Mode
					echo[1].SetDelayTime(midi_clk_detect.GetSyncDelay()); //Midi Sync Mode
				} else {
					echo[0].SetDelayTime(hw.ProcessXaxis()); // Free Time Mode
					echo[1].SetDelayTime(hw.ProcessXaxis()); // Free Time Mode
				}
				echo[0].SetFeedback(hw.ProcessYaxis());
				echo[1].SetFeedback(hw.ProcessYaxis());
				break;
			case DECIMATOR :
				LoFi[0].SetDownsampleFactor(hw.ProcessXaxis());
				LoFi[1].SetDownsampleFactor(hw.ProcessXaxis());
				LoFi[0].SetBitcrushFactor(hw.ProcessYaxis());
				LoFi[1].SetBitcrushFactor(hw.ProcessYaxis());

				break;
			case HPF_DELAY :
				filter[0].SetFreq(hw.ProcessXaxis());
				filter[1].SetFreq(hw.ProcessXaxis());
				if (ClkSync) {
					midi_clk_detect.SetSyncDelay(static_cast<midi_clock_handler::SyncNoteValue>(std::floor(hw.ProcessYaxis()))); //Midi Sync Mode
					echo[0].SetDelayTime(midi_clk_detect.GetSyncDelay()); //Midi Sync Mode
					echo[1].SetDelayTime(midi_clk_detect.GetSyncDelay()); //Midi Sync Mode
				} else {
					echo[0].SetDelayTime(hw.ProcessYaxis()); // Free Time Mode
					echo[1].SetDelayTime(hw.ProcessYaxis()); // Free Time Mode
				}
				break;
			case TREMOLO :
				if (ClkSync) {
					midi_clk_detect.SetSyncDelay(static_cast<midi_clock_handler::SyncNoteValue>(std::floor(hw.ProcessXaxis()))); //Midi Sync Mode
					trem[0].SetFreq(midi_clk_detect.GetSyncFreq());
					trem[1].SetFreq(midi_clk_detect.GetSyncFreq());
				} else {
					trem[0].SetFreq(hw.ProcessXaxis());
					trem[1].SetFreq(hw.ProcessXaxis());
				}
				trem[0].SetDepth(hw.ProcessYaxis());
				trem[1].SetDepth(hw.ProcessYaxis());
				break;
			default : ;
		}

		hw.SetDispMode(DaisyPad::XY_POS);
		param_change_time = System::GetNow();
	} 
	
	// - - - - Knobs - - - -
	if (abs(hw.knob1.Value() - knob1_prev) > 0.01f) {
		knob1_prev = hw.knob1.Value();

		switch (active_fx) {
			case BPF :
				filter[0].SetInputDrive(2*knob1_exp.Process());
				filter[1].SetInputDrive(2*knob1_exp.Process());
				break;
			case DELAY :
				echo[0].SetTone(knob1_exp.Process());
				echo[1].SetTone(knob1_exp.Process());
				break;
			case DECIMATOR :
				break;
			case HPF_DELAY : 
				echo[0].SetFeedback(hw.knob1.Value());
				echo[1].SetFeedback(hw.knob1.Value());
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
			case BPF :
				break;
			case DELAY :
				break;
			case DECIMATOR :
				break;
			case HPF_DELAY :
				break;
			case TREMOLO :
				break;
			default : ;
		}

		hw.SetDispMode(DaisyPad::PARAM2);
		param_change_time = System::GetNow();
	}

	// - - - - Push Buttons - - - -
	if (hw.button1.RisingEdge()) {
		knob1_prev = hw.knob1.Value();

		switch (active_fx) {
			case BPF :
				type24db = !type24db;
				if (type24db) {
					filter[0].SetFilterMode(LadderFilter::FilterMode::BP24);
					filter[1].SetFilterMode(LadderFilter::FilterMode::BP24);
					hw.SetTemp7SegCode(0x5B665e7c); //24db
				} else {
					filter[0].SetFilterMode(LadderFilter::FilterMode::BP12);
					filter[1].SetFilterMode(LadderFilter::FilterMode::BP12);
					hw.SetTemp7SegCode(0x065B5e7c); //12db
				}
				break;
			case DELAY :
				ClkSync = !ClkSync;
				if (ClkSync) {
					hw.SetTemp7SegCode(0x6D6E5458); //Sync
					hw.XAxisParam(0, 8, DaisyPad::LINEAR); //Midi Sync Mode
					hw.SetBPM(midi_clk_detect.ClockBPM());
				} else {
					hw.SetTemp7SegCode(0x71507979); //Free
					hw.XAxisParam(MIN_DELAY, MAX_DELAY, DaisyPad::EXPONENTIAL); // Free Time Mode
				}
				break;
			case DECIMATOR :
				SmoothCrush = !SmoothCrush;
				if (SmoothCrush) {
					LoFi[0].SetSmoothCrushing(true);
					LoFi[1].SetSmoothCrushing(true);
				} else {
					LoFi[0].SetSmoothCrushing(false);
					LoFi[1].SetSmoothCrushing(false);
				}
				break;
			case HPF_DELAY :
				ClkSync = !ClkSync;
				if (ClkSync) {
					hw.SetTemp7SegCode(0x6D6E5458); //Sync
					hw.YAxisParam(0, 8, DaisyPad::LINEAR); //Midi Sync Mode
					hw.SetBPM(midi_clk_detect.ClockBPM());
				} else {
					hw.SetTemp7SegCode(0x71507979); //Free
					hw.YAxisParam(MIN_DELAY, MAX_DELAY, DaisyPad::EXPONENTIAL); // Free Time Mode
				}
				break;
			case TREMOLO :
				ClkSync = !ClkSync;
				if (ClkSync) {
					hw.SetTemp7SegCode(0x6D6E5458); //Sync
					hw.XAxisParam(0, 8, DaisyPad::LINEAR); //Midi Sync Mode
					hw.SetBPM(midi_clk_detect.ClockBPM());
				} else {
					hw.SetTemp7SegCode(0x71507979); //Free
					hw.XAxisParam(0.25f, 20.0f, DaisyPad::EXPONENTIAL); // Free Time Mode
				}
				break;
			default : ;
		}

		param_change_time = System::GetNow();
	}
	if (hw.button2.RisingEdge()) {

		switch (active_fx) {
			case BPF :
				break;
			case DELAY :
				break;
			case DECIMATOR :
				break;
			case HPF_DELAY :
				break;
			case TREMOLO :
				break;
			default : ;
		}

		param_change_time = System::GetNow();
	}

	// - - - - Rotary Encoder Button - - - -
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
			
			case BPF :
				if (!hw.touch_detect.Read()) {
					out[0][i] = filter[0].Process(in[0][i]);
					out[1][i] = filter[1].Process(in[1][i]);
				} else {
					out[0][i] = in[0][i];
					out[1][i] = in[1][i];
				}
				break;
			case DELAY :
				out[0][i] = echo[0].Process(in[0][i]);
				out[1][i] = echo[1].Process(in[1][i]);
				break;
			case DECIMATOR : 
				out[0][i] = LoFi[0].Process(in[0][i]);
				out[1][i] = LoFi[1].Process(in[1][i]);
				break;
			case HPF_DELAY :
				if (!hw.touch_detect.Read()) {
					out[0][i] = echo[0].Process(filter[0].Process(in[0][i]));
					out[1][i] = echo[1].Process(filter[1].Process(in[1][i]));
				} else {
					out[0][i] = in[0][i] + echo[0].Process(0);
					out[1][i] = in[1][i] + echo[1].Process(0);
				}
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

void handle_midi_rt(MidiEvent& m) {

	if (m.srt_type == TimingClock) {
		midi_clk_detect.ClockReceived();
	} else if (m.srt_type == Start || m.srt_type == Continue) {
		midi_clk_detect.StartReceived();
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
	
	filter[0].Init(hw.AudioSampleRate());
	filter[0].SetFilterMode(LadderFilter::FilterMode::BP12);
	filter[1].Init(hw.AudioSampleRate());
	filter[1].SetFilterMode(LadderFilter::FilterMode::BP12);
	delmem[0].Init();
	delmem[1].Init();
	echo[0].Init(hw.AudioSampleRate(), &delmem[0]);
	echo[1].Init(hw.AudioSampleRate(), &delmem[1]);
	LoFi[0].Init();
	LoFi[1].Init();
	trem[0].Init(hw.AudioSampleRate());
	trem[1].Init(hw.AudioSampleRate());

	knob1_exp.Init(hw.knob1, 0, 1, Parameter::EXPONENTIAL);

	midi_clk_detect.Init(hw.AudioSampleRate());
	hw.midi.StartReceiveRt(&handle_midi_rt);

	hw.StartAudio(AudioCallback);
	hw.StartAdc();
	
	while(1) {

		//Handle TouchScreen and LED Matrix
		hw.ProcessPeripherals();

		//Clock Stuff
		hw.SetPulseCorners(midi_clk_detect.IsQuarterNote() && ClkSync);

		
		//Return Display to Display Code after 2 Seconds
		if ((hw.GetDispMode() != DaisyPad::DISP_CODE) && (hw.GetDispMode() != DaisyPad::CLEAR)) {
			if (System::GetNow() - param_change_time > 2000) {
				hw.SetDispMode(DaisyPad::DISP_CODE);
			}
		}
	}
}
