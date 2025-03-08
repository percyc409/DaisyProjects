#include "daisy_pod.h"
#include "daisysp.h"
#include "core_cm7.h"
#include "STFT_Base_test.h"

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

#define WINDOW_SIZE 1024
#define LAPS 4

bool bypass = true;
bool print_times = false;
uint32_t fft_run_time; //Cycles taken by audio callback when FFT is conducted
uint32_t std_run_time; //Cycles taken by audio callback when FFT is not conducted

STFT_Base_test<WINDOW_SIZE, LAPS> FFT;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	//Measure - start
	DWT->CYCCNT = 0;

	//Controls
	hw.ProcessAllControls();
	print_times = (hw.button2.RisingEdge()) ? true : print_times;
	
	if(hw.button1.RisingEdge()) {
		bypass = !bypass;

		if (bypass) {
			hw.led1.Set(0, 0, 0); // off
		} else {
			hw.led1.Set(0, 0, 1); // blue
		}

		hw.UpdateLeds();
	}

	//Audio
	for (size_t i = 0; i < size; i++) {
		
		if (bypass) {
			out[0][i] = in[0][i];
			out[1][i] = in[1][i];
		}
		else {	
			FFT.Process(in[0][i], out[0][i]);
		}
	}

	//Measure - end
	if (FFT.fft_ran_check()) {
		fft_run_time = DWT->CYCCNT;
	} else {
		std_run_time = DWT->CYCCNT;
	}
	
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(48); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartAdc();

	// Enable Logging, and set up the USB connection.
 	hw.seed.StartLog(true);

	// setup measurement
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->LAR = 0xC5ACCE55;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	fft_run_time = 0;
	std_run_time = 0;

	hw.StartAudio(AudioCallback);

	while(1) {

		if (print_times) {
			hw.seed.PrintLine("Audio Callback run time without FFT proc: %d Cycles", std_run_time);
			hw.seed.PrintLine("Audio Callback run time with FFT proc: %d Cycles\n", fft_run_time);
			print_times = false;
		}
		
		System::Delay(1000);

	}
}


