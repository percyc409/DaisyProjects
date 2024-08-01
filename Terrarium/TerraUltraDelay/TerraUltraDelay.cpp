#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "taptempo.h"
#include "nfctsm_pitch.h"
#include "delayline_reverse.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

#define MAX_DELAY static_cast<size_t>(48000 * 1.8f)  // 2.2s in samples
#define MIN_DELAY static_cast<size_t>(48000 * 0.1f)  // 100ms in samples
#define MAX_MOD    48000 * 0.003f                    // 3ms in samples
#define MIN_OFFSET 48000 * 0.003f                    // 5ms in samples
#define MAX_OFFSET 48000 * 0.025f                    // 25ms in samples
#define DELAYLINE_LEN static_cast<size_t>(MAX_DELAY+MAX_OFFSET+MAX_MOD)


DaisyPetal hw;
dsy_gpio led1;
dsy_gpio led2;

DelayLine<float, DELAYLINE_LEN> DSY_SDRAM_BSS delmem;
DelayLineReverse<float, DELAYLINE_LEN> DSY_SDRAM_BSS rev_delmem;
Tap_Tempo<MIN_DELAY, MAX_DELAY> time_tt;
Oscillator lfo;
nfctsm_pitch PitchShift;
OnePole delay_lp;

float lfo_val;


// Struct to define Modulated Delay struct

struct mod_delay
{
    DelayLine<float, DELAYLINE_LEN> *del;
	DelayLineReverse<float, DELAYLINE_LEN> *rev_del;
	
    float currentDelay;
    float delayTarget;
    float feedback;
	float offset;
	bool chorus; // Vibrato or Chorus/Flanger switch
	bool rev_en;

    float Process(float in)
    {
        //set delay times
        fonepole(currentDelay, delayTarget, .0002f);
        del->SetDelay(currentDelay);
		rev_del->SetDelay1(currentDelay);

		float fx_out;
		float rev_out = 0.0f;

		if (rev_en) {
			rev_out = feedback * rev_del->ReadRev();
			rev_del->Write(in);
		}
        
		if (chorus) {

			float dry_read = del->Read(currentDelay);
			float wet_read = del->Read(currentDelay + offset + lfo_val);
			fx_out = feedback * 0.6f * (dry_read + wet_read);

		} else {
			
			fx_out = feedback * del->Read(currentDelay + lfo_val);

		}

		if (rev_en) {
        	del->Write(fx_out + rev_out);
		} else {
			del->Write(fx_out + in);
		}

        return fx_out;
    }
};



mod_delay delay_fx;

Parameter time;
Parameter feedback;
Parameter mix;
Parameter modRate;
Parameter modDepth;
Parameter Pitch;

float time_old;
bool bypass = true;
bool ps_en;
bool fb_shift;

void ProcessControls() {
	
	hw.ProcessAllControls();
	time_tt.Process();

	//knobs
	delay_fx.feedback = feedback.Process();
	mix.Process();
	lfo.SetAmp(modDepth.Process());
	lfo.SetFreq(modRate.Process());

	PitchShift.SetSemitones(static_cast<int>(round(Pitch.Process())));

	if(time_tt.Use_tt()) {
		delay_fx.delayTarget = time_tt.Count_Avg();

		if (abs (time_old - time.Process()) > 8000)
			time_tt.Stop_tt();

	} else
		delay_fx.delayTarget = time.Process();

	//switches

	if(hw.switches[Terrarium::SWITCH_1].Pressed()) 
        delay_fx.chorus = true;
    else
		delay_fx.chorus = false;

	if(hw.switches[Terrarium::SWITCH_2].Pressed()) 
        delay_fx.rev_en = true;
    else
		delay_fx.rev_en = false;

	if(hw.switches[Terrarium::SWITCH_3].Pressed()) 
        ps_en = true;
    else
		ps_en = false;

	if(hw.switches[Terrarium::SWITCH_4].Pressed()) 
        delay_fx.chorus = true;
    else
		delay_fx.chorus = false;

	//footswitch
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
    }
	
	if(hw.switches[Terrarium::FOOTSWITCH_2].RisingEdge())
    {
        time_tt.Tap();
		time_old = time.Value();
    }
	
	//LEDs
	dsy_gpio_write(&led1, !bypass);
	dsy_gpio_write(&led2, time_tt.Tapping());

}


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	ProcessControls();

	float fx_in, fx_out;

	for (size_t i = 0; i < size; i++) {

		lfo_val = lfo.Process();
		
		if (bypass) {
			out[0][i] = in[0][i];
		}
		else {

			fx_in = in[0][i];

			if (ps_en) {
				fx_in = PitchShift.Process(fx_in);
				fx_in = delay_lp.Process(fx_in);
			} 
			
			fx_out = delay_fx.Process(fx_in);
			
			out[0][i] = in[0][i] + fx_out;
		}
	}
}

void Init() {

	delmem.Init();
	rev_delmem.Init();
	delay_fx.del = &delmem;
	delay_fx.rev_del = &rev_delmem;
	delay_fx.offset = MAX_OFFSET;

	delay_lp.Init();
	delay_lp.SetFrequency(0.0625f);

	time.Init(hw.knob[Terrarium::KNOB_1], MIN_DELAY, MAX_DELAY, Parameter::EXPONENTIAL);
	feedback.Init(hw.knob[Terrarium::KNOB_2], 0.0f, 1.0f, Parameter::EXPONENTIAL);
	mix.Init(hw.knob[Terrarium::KNOB_3], 0.0f, 1.0f, Parameter::EXPONENTIAL);
	modRate.Init(hw.knob[Terrarium::KNOB_4], .6f, 10.0f, Parameter::EXPONENTIAL);
	modDepth.Init(hw.knob[Terrarium::KNOB_5], 0.0f, MAX_MOD, Parameter::EXPONENTIAL);
	Pitch.Init(hw.knob[Terrarium::KNOB_6], -12.0f, 12.0f, Parameter::LINEAR);

	time_tt.Init(hw.AudioSampleRate(), hw.AudioBlockSize());
	PitchShift.Init();

	lfo.Init(hw.AudioSampleRate());
	lfo.SetWaveform(lfo.WAVE_SIN);

}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(48); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	Init();

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
