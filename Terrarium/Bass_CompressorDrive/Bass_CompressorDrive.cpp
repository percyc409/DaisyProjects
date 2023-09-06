#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include <string>

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal hw;
dsy_gpio led1;
dsy_gpio led2;

Compressor comp;
Svf filter; // State Variable filter
Overdrive overdrive;

Parameter ratio;
Parameter threshold;
Parameter makeup;
Parameter attack;
Parameter release;

Parameter mix; // between distorted and clean sig (Ignored when overdrive is off)
Parameter drive;
Parameter cutoff;

bool bypassComp = true;
bool bypassDrive = true;
bool autogain = false;
bool last_autogain = false;
bool lpDry;

enum modes {compress, od}; // K5+K6 Mode
modes knobmode;
modes ledmode;

void ProcessControls() {

	hw.ProcessAllControls();
	
    //Single-Parameter Knobs
    comp.SetThreshold(threshold.Process());
    comp.SetRatio(ratio.Process());
	comp.SetMakeup(makeup.Process());

    //Multi-Parameter Knobs
    switch(knobmode){

        case compress:  comp.SetAttack(attack.Process());
                        comp.SetRelease(release.Process());
                        break;
        
        case od:        filter.SetFreq(cutoff.Process());
                        overdrive.SetDrive(drive.Process());
                        break;
    }

    //switches
    knobmode = (hw.switches[Terrarium::SWITCH_1].Pressed()) ? od : compress;
    ledmode = (hw.switches[Terrarium::SWITCH_2].Pressed()) ? compress : od;
    lpDry = (hw.switches[Terrarium::SWITCH_3].Pressed()) ? true : false;

	//footswitch
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypassComp = !bypassComp;
    }

    if(hw.switches[Terrarium::FOOTSWITCH_2].RisingEdge())
    {
        bypassDrive = !bypassDrive;
    }
	
    //Led1
	dsy_gpio_write(&led1, !bypassComp);

    //Led2
    float gain;

    switch(ledmode){

        case compress:  //Compression Indicator
                        gain = comp.GetGain();

                        if (gain - makeup.Process() < -0.5f) //Checking whether applied gain  is less than -0.5dB (Without makeup gain)
                            dsy_gpio_write(&led2, true);
                        else 
                            dsy_gpio_write(&led2, false);

                        break;
        
        case od:        dsy_gpio_write(&led2, !bypassDrive);
                        break;
    }

}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float sig, hp, lp, drive_out;
	ProcessControls();

    for(size_t i = 0; i < size; i++) {
        
		if(bypassComp) {
            sig = in[0][i];	
        }
        else {
			// Scales input by 2 and then the output by 0.5
    		// This is because there are 6dB of headroom on the daisy
    		// and currently no way to tell where '0dB' is supposed to be
			sig = comp.Process(in[0][i] * 2.0f) * 0.5f;     
        }

        if(bypassDrive) {
            out[0][i] = sig;	
        }
        else {
			// Parrallel Process Lo and Hi sigs
    		// Hi sigs are passed to overdrive
    	
			filter.Process(sig);
            hp = filter.High();
            lp = filter.Low();

            drive_out = overdrive.Process(hp);

            if(lpDry) {
                out[0][i]  = drive_out * mix.Process() + (1-mix.Process()) * lp;
            }
            else {
                out[0][i]  = drive_out * mix.Process() + (1-mix.Process()) * sig;
            }

            
        }
    }
}

void Init(float samplerate)
{
    comp.Init(samplerate);
	comp.AutoMakeup(false);
    overdrive.Init();
    filter.Init(samplerate);
    filter.SetFreq(500.0f);
    filter.SetRes(0.25f);

    attack.Init(hw.knob[Terrarium::KNOB_5], 0.01f, 1.0f, Parameter::EXPONENTIAL);
    release.Init(hw.knob[Terrarium::KNOB_6], 0.01f, 1.0f, Parameter::EXPONENTIAL);
    ratio.Init(hw.knob[Terrarium::KNOB_2], 1.0f, 40.0f, Parameter::EXPONENTIAL);
    threshold.Init(hw.knob[Terrarium::KNOB_1], -50.0f, 0.0f, Parameter::LINEAR);
    makeup.Init(hw.knob[Terrarium::KNOB_3], 1.0f, 40.0f, Parameter::LINEAR);  // log?

    mix.Init(hw.knob[Terrarium::KNOB_4], 0.00f, 1.0f, Parameter::LINEAR);
    drive.Init(hw.knob[Terrarium::KNOB_5], 0.0f, 0.6f, Parameter::LINEAR);
    cutoff.Init(hw.knob[Terrarium::KNOB_6], 50.0f, 2000.0f, Parameter::EXPONENTIAL);
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	Init(hw.AudioSampleRate());
	hw.StartAdc();
	hw.StartAudio(AudioCallback);

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
