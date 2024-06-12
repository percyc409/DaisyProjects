#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "taptempo.h"

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
Tap_Tempo<MIN_DELAY, MAX_DELAY> time_tt;
Oscillator lfo;

float lfo_val;


// Struct to define Modulated Delay struct

struct mod_delay
{
    DelayLine<float, DELAYLINE_LEN> *del;
    float currentDelay;
    float delayTarget;
    float feedback;
	float offset;
	bool chorus; // Vibrato or Chorus/Flanger switch

    float Process(float in)
    {
        //set delay times
        fonepole(currentDelay, delayTarget, .0002f);
        del->SetDelay(currentDelay);

		float fx_out;
        
		if (chorus) {

			float dry_read = del->Read(currentDelay);
			float wet_read = del->Read(currentDelay + offset + lfo_val);
			fx_out = feedback * 0.6f * (dry_read + wet_read);

		} else {
			
			fx_out = feedback * del->Read(currentDelay + lfo_val);

		}

        del->Write(fx_out + in);

        return fx_out;
    }
};



mod_delay delay_fx;

Parameter time;
Parameter feedback;
Parameter mix;
Parameter modRate;
Parameter modDepth;
Parameter modOffset;

int time_old;
bool bypass = true;

void ProcessControls() {
	
	hw.ProcessAllControls();
	time_tt.Process();

	//knobs
	delay_fx.feedback = feedback.Process();
	delay_fx.offset = modOffset.Process();
	mix.Process();
	lfo.SetAmp(modDepth.Process());
	lfo.SetFreq(modRate.Process());

	if(time_tt.Use_tt()) {
		delay_fx.delayTarget = time_tt.Count_Avg();

		if (abs (time_old - time.Process()) > 8000)
			time_tt.Stop_tt();

	} else
		delay_fx.delayTarget = time.Process();

	//switches

	if(hw.switches[Terrarium::SWITCH_2].Pressed()) 
        lfo.SetWaveform(lfo.WAVE_TRI);
    else
		lfo.SetWaveform(lfo.WAVE_SIN);

	if(hw.switches[Terrarium::SWITCH_1].Pressed()) 
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

	float del_out;

	for (size_t i = 0; i < size; i++) {

		lfo_val = lfo.Process();
		
		if (bypass) {
			out[0][i] = in[0][i];
		}
		else {
			
			del_out = delay_fx.Process(in[0][i]);

			out[0][i] = in[0][i] + mix.Value() * del_out;
		}
	}
}

void Init() {

	delmem.Init();
	delay_fx.del = &delmem;

	time.Init(hw.knob[Terrarium::KNOB_1], MIN_DELAY, MAX_DELAY, Parameter::EXPONENTIAL);
	feedback.Init(hw.knob[Terrarium::KNOB_2], 0.0f, 1.0f, Parameter::EXPONENTIAL);
	mix.Init(hw.knob[Terrarium::KNOB_3], 0.0f, 1.0f, Parameter::EXPONENTIAL);
	modRate.Init(hw.knob[Terrarium::KNOB_4], .6f, 10.0f, Parameter::EXPONENTIAL);
	modDepth.Init(hw.knob[Terrarium::KNOB_5], 0.0f, MAX_MOD, Parameter::EXPONENTIAL);
	modOffset.Init(hw.knob[Terrarium::KNOB_6], MIN_OFFSET, MAX_OFFSET, Parameter::LINEAR);

	time_tt.Init(hw.AudioSampleRate(), hw.AudioBlockSize());

	lfo.Init(hw.AudioSampleRate());
	lfo.SetWaveform(lfo.WAVE_SIN);

}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
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
