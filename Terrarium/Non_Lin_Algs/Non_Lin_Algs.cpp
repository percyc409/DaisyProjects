#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"
#include "highshelf.h"
#include "arm_math.h"
#include "polyphaseFIRcoefs.h"


using namespace daisy;
using namespace daisysp;
using namespace terrarium;

#define NUM_TAPS 96
#define BLOCK_SIZE 4
#define L 4 // Also M

DaisyPetal hw;
dsy_gpio led1;
dsy_gpio led2;

//Oversampling<4,1> Oversampling_o;
Highshelf Pre_emphasis, Post_filt;

arm_fir_interpolate_instance_f32 upsampler_i;
arm_fir_decimate_instance_f32 downsampler_i;

static float upsamp_state[(NUM_TAPS/L)+BLOCK_SIZE-1];
static float downsamp_state[NUM_TAPS+(BLOCK_SIZE*L)-1];
static float ovrsmp_arr[L * BLOCK_SIZE];

Overdrive overdrive;

Parameter gain;
Parameter volume;
Parameter driveLevel; // between distorted and clean sig (Ignored when overdrive is off)
Parameter drive;

int nl_alg;
bool bypass = true;
bool dsy_drive;


void ProcessControls() {
	
	hw.ProcessAllControls();

	//knobs
	gain.Process();
	volume.Process();
  	overdrive.SetDrive(drive.Process());
    driveLevel.Process();

	//switches
	nl_alg = (hw.switches[Terrarium::SWITCH_2].Pressed()) ? 1 : 0;
	
	if ((hw.switches[Terrarium::SWITCH_1].Pressed())) 
		nl_alg += 2;

	dsy_drive = (hw.switches[Terrarium::SWITCH_3].Pressed()) ? true : false;

	//footswitches
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
    }

	//leds
	dsy_gpio_write(&led1, !bypass);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//Non-Linear Algorithms
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SoftClip(float in, float &out) {
	out = tanh(in);
}

void HardClip(float in, float &out) { 
	out = in > 1.f ? 1.f : in;
    out = in < -1.f ? -1.f : in;
}

void Fuzz(float in, float &out) {
	out = in >=0 ? in : in/(gain.Value()); 
}

void WaveFold(float in, float &out) {
	out = sin(in);
}

float NonLinearProcess(float in){

	float in_g = in * gain.Value();
	float out = 0.0f;

	switch(nl_alg) {

		case 0: SoftClip(in_g, out);
				break;

		case 1: HardClip(in_g, out);
				break;

		case 2: Fuzz(in_g, out);
				break;

		case 3: WaveFold(in_g, out);
				break;
	}

	return out * volume.Value();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	ProcessControls();

	if (bypass) {
		for (size_t i = 0; i < size; i++) 
			out[0][i] = in[0][i];
	}
	else {
	
		float mono_in[BLOCK_SIZE] = {};
		float mono_out[BLOCK_SIZE] = {};
		
		for (int i = 0; i<BLOCK_SIZE; i++)
			mono_in[i] = in[0][i];


		//Upsample
		arm_fir_interpolate_f32(&upsampler_i, mono_in, ovrsmp_arr, BLOCK_SIZE);


		for (int i = 0; i < L * BLOCK_SIZE; i++) {
			
			ovrsmp_arr[i] = Pre_emphasis.Process(ovrsmp_arr[i]);

			if(dsy_drive)
				ovrsmp_arr[i] = overdrive.Process(ovrsmp_arr[i]) * driveLevel.Value();
			else
				ovrsmp_arr[i] = NonLinearProcess(ovrsmp_arr[i]);

			ovrsmp_arr[i] = Post_filt.Process(ovrsmp_arr[i]);

		}

		//Downsample
		arm_fir_decimate_f32(&downsampler_i, ovrsmp_arr, mono_out, BLOCK_SIZE*L);

		for (int i = 0; i<BLOCK_SIZE; i++)
			out[0][i] = mono_out[i];

	}	
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	//Oversampling_o.Init();
	Pre_emphasis.Init(hw.AudioSampleRate() * L);
	Post_filt.Init(hw.AudioSampleRate() * L);
	Pre_emphasis.SetFreq(8000.0f);
	Post_filt.SetFreq(8000.0f);
	Pre_emphasis.SetGain(36.0F);
	Post_filt.SetGain(-36.0F);
	overdrive.Init();

	gain.Init(hw.knob[Terrarium::KNOB_1], 1.0f, 30.0f, Parameter::LINEAR);
	volume.Init(hw.knob[Terrarium::KNOB_2], 0.0f, 1.5f, Parameter::EXPONENTIAL);
	driveLevel.Init(hw.knob[Terrarium::KNOB_5], 0.00f, 1.0f, Parameter::EXPONENTIAL);
    drive.Init(hw.knob[Terrarium::KNOB_4], 0.0f, 0.8f, Parameter::LINEAR);

	arm_fir_interpolate_init_f32(&upsampler_i, L, NUM_TAPS, (float *)&polyphaseFIRcoefs_4_reversed[0], &upsamp_state[0],BLOCK_SIZE);
	arm_fir_decimate_init_f32(&downsampler_i, NUM_TAPS, L, (float *)&polyphaseFIRcoefs_4_reversed[0], &downsamp_state[0],BLOCK_SIZE*L);

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

	
	while(1) {

	}
}
