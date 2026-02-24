#include "daisy_pod.h"
#include "daisysp.h"
#include "../../Utils/shy_fft.h"
#include "stmlib/atan.h"

#define PI 3.1415926535897932384626433832795
#define PI_2 1.5707963267948966192313f
#define TWO_PI 6.28318530718f

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

#define WINDOW_SIZE 1024
#define STRIDE 1024

ShyFFT<float, WINDOW_SIZE, LutPhasor> fft;
Oscillator osc;

Parameter freq;
Parameter amp;

float fft_in[WINDOW_SIZE * 2];
float fft_out[WINDOW_SIZE];
float magnitude0[WINDOW_SIZE/2];
float magnitude1[WINDOW_SIZE/2];
float bin_centre_frequency[WINDOW_SIZE/2];

float osc_freq;

int wr_ptr = 0;
bool reading = false;
bool print_results = false;
bool scnd_win = false;

float max_mag_val0;
float max_i_val0;
float max_q_val0;
size_t max_mag_idx0, max_mag_idx1;
float max_i_val1;
float max_q_val1;
float win1_mid_val, win2_mid_val;


float phase2dev = -(float)WINDOW_SIZE / ((float)STRIDE * TWO_PI);
float dev2phase = (TWO_PI * (float)STRIDE) / (float)WINDOW_SIZE;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	
	if (hw.button1.RisingEdge()) {
		osc.Reset();
		reading = true;
	}
	osc_freq = freq.Process();
	osc.SetFreq(osc_freq);
	osc.SetAmp(amp.Process());

	for (size_t i = 0; i < size; i++)
	{

		if (reading) {

			if (wr_ptr == 0) {
				osc_freq = freq.Process();
				//osc_freq = (1*48000.0f)/1024.0f;
				osc.SetFreq(osc_freq);
				max_mag_val0 = 0.0f;
				max_i_val0 = 0.0f;
				max_q_val0 = 0.0f;
				max_mag_idx0 = 0;
				scnd_win = false;
			} else if (wr_ptr == WINDOW_SIZE) {
				//osc.Reset(0.25);
				//osc_freq = (2*48000.0f)/1024.0f;
				osc.SetFreq(osc_freq);
				max_mag_val0 = 0.0f;
			}

			float in = osc.Process();
			fft_in[wr_ptr] = in;
			wr_ptr++;

			if(wr_ptr==WINDOW_SIZE/2) {
				win1_mid_val = in;
			} else if (wr_ptr == WINDOW_SIZE + WINDOW_SIZE/2) {
				win2_mid_val = in;
			}

			//Window Full
			if (wr_ptr % WINDOW_SIZE == 0) {
				
				// FFT Launch
				if (scnd_win) {
					fft.Direct((fft_in + WINDOW_SIZE), fft_out);
				} else {
					fft.Direct(fft_in, fft_out);
				}
				
				//Magnitude Calculation
				if (scnd_win) {
					for (size_t j = 0; j < WINDOW_SIZE/2; j++) {
						magnitude1[j] = sqrt(fft_out[j + WINDOW_SIZE/2] * fft_out[j + WINDOW_SIZE/2] + fft_out[j] * fft_out[j]);

						if  (magnitude1[j] > max_mag_val0) {
							max_mag_val0 = magnitude1[j];
							max_mag_idx1 = j;
							max_i_val1 = fft_out[j];
							max_q_val1 = fft_out[j + WINDOW_SIZE/2];
						}
					}

					scnd_win = false;
					print_results = true;
					wr_ptr = 0;
					reading = false;

				} else {
					for (size_t j = 0; j < WINDOW_SIZE/2; j++) {
						magnitude0[j] = sqrt(fft_out[j + WINDOW_SIZE/2] * fft_out[j + WINDOW_SIZE/2] + fft_out[j] * fft_out[j]);

						if  (magnitude0[j] > max_mag_val0) {
							max_mag_val0 = magnitude0[j];
							max_mag_idx0 = j;
							max_i_val0 = fft_out[j];
							max_q_val0 = fft_out[j + WINDOW_SIZE/2];
						}
					}

					scnd_win = true;

				}

			}
		}

		out[0][i] = in[0][i];
		out[1][i] = in[1][i];
	}
}

// Wrap the phase to the range -pi to pi
float wrapPhase(float phaseIn)
{
	if (phaseIn >= 0)
		return fmodf(phaseIn + PI, TWO_PI) - PI;
	else
		return fmodf(phaseIn - PI, -TWO_PI) + PI;	
}

// Polynomial approximating arctangenet on the range -1,1.
// Max error < 0.005 (or 0.29 degrees)
float atan_approx(float z)
{
	const float n1 = 0.97239411f;
	const float n2 = -0.19194795f;
	return (n1 + n2 * z * z) * z;
}


float atan2_approx(float y, float x){
	float ay = fabs(y), ax = fabs(x);
	int invert = ay > ax;
	float z = invert ? ax/ay : ay/ax; // [0,1]
	float th = atan_approx(z);        // [0,π/4]
	if(invert) th = PI_2 - th;       // [0,π/2]
	if(x < 0) th = PI - th;          // [0,π]
	th = copysign(th, y);              // [-π,π]
	return th;
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(48); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	fft.Init();
	osc.Init(hw.AudioSampleRate());
	freq.Init(hw.knob1, 1.0f, 24000.0f, Parameter::LINEAR);
	amp.Init(hw.knob2, 0.0f, 1.0f, Parameter::LINEAR);

	for (size_t n = 0; n < WINDOW_SIZE/2; n++){
		bin_centre_frequency[n] = (TWO_PI * fmodf((float)n * (float)STRIDE / (float)WINDOW_SIZE, 1.0f));
	}

	osc_freq = freq.Process();

	// Enable Logging, and set up the USB connection.
	hw.seed.StartLog();

	hw.StartAdc();
	hw.StartAudio(AudioCallback);
	while(1) {

		if (print_results){
			print_results = false;

			float phase0, phase1, test0, test1;
			float phase_diff, pd2;
			float phase_diff_wrap;
			float test_phase_diff, test_phase_diff_wrap;
			float bin_deviation, bd2;
			float analysis_freq, synth_freq;
			float target_freq_bin;
			size_t new_bin;
			float target_synth_freq, target_synth_bin;

			//phase0 = stmlib::fast_atan2(max_q_val0, max_i_val0) /10000.0f;
			//phase1 = stmlib::fast_atan2(max_q_val1, max_i_val1) /10000.0f;
			phase0 = atan2_approx(max_q_val0, max_i_val0);
			phase1 = atan2_approx(max_q_val1, max_i_val1);

			test0 = atan2_approx(max_i_val0, max_q_val0);
			test1 = atan2_approx(max_i_val1, max_q_val1);


			phase_diff = phase1 - phase0;
			test_phase_diff = test1 - test0;
            phase_diff_wrap = wrapPhase(phase_diff);
			test_phase_diff_wrap = wrapPhase(test_phase_diff);
			bin_deviation = phase_diff_wrap * phase2dev;
			analysis_freq = (float)max_mag_idx0 + bin_deviation;
			target_freq_bin = osc_freq/(48000.0f/WINDOW_SIZE);

			new_bin = static_cast<size_t>(floorf(analysis_freq * 1.5f + 0.5f));
			synth_freq = analysis_freq * 1.5f;

			bd2 = synth_freq - new_bin;
			pd2 = bd2 * dev2phase;

			target_synth_freq = osc_freq * 1.5f;
			target_synth_bin = target_synth_freq/(48000.0f/WINDOW_SIZE);



			hw.seed.PrintLine("* * * * * * * * * * * * * * * * * * * * * * * *");
			hw.seed.PrintLine("Oscillator Frequency: %f Hz", osc_freq);
			hw.seed.PrintLine("Max mag idx: %d, Centre Frequency: %f", max_mag_idx0, max_mag_idx0*48000.0f/1024.0f);
			hw.seed.PrintLine("Max mag idx: %d, Centre Frequency: %f", max_mag_idx1, max_mag_idx1*48000.0f/1024.0f);
			//hw.seed.PrintLine("Max mag value window 0: %f", max_mag_val0);
			hw.seed.PrintLine("Max Bin content window 0: %f + %fi\n", max_i_val0, max_q_val0);
			hw.seed.PrintLine("Max Bin content window 1: %f + %fi\n", max_i_val1, max_q_val1);

			hw.seed.PrintLine("Phase win0: %f, Phase win1: %f", phase0, phase1);
			hw.seed.PrintLine("Phase win0: %f, Phase win1: %f", test0, test1);
			hw.seed.PrintLine("Phase diff: %f, Phase diff wrap: %f", phase_diff, phase_diff_wrap);
			hw.seed.PrintLine("Test Phase diff: %f, Test Phase diff wrap: %f", test_phase_diff, test_phase_diff_wrap);
			hw.seed.PrintLine("Bin Deviation: %f", bin_deviation);
			hw.seed.PrintLine("Analysis Freq bin: %f", analysis_freq);
			hw.seed.PrintLine("Target Freq bin: %f", target_freq_bin);

			hw.seed.PrintLine("New bin: %d, Synth freq: %f", new_bin, synth_freq);
			hw.seed.PrintLine("Target Synth Freq: %f, Target synth Bin: %f", target_synth_freq, target_synth_bin);
			hw.seed.PrintLine("Bin Dev out: %f, Phase diff out: %f", bd2, pd2);

			//hw.seed.PrintLine("Middle Window 0 Osc value: %f", win1_mid_val);
			//hw.seed.PrintLine("Middle Window 1 Osc value: %f", win2_mid_val);
		}

		System::Delay(1000);

	}
}
