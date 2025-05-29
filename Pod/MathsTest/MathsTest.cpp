#include "daisy_pod.h"
#include "daisysp.h"
#include "core_cm7.h"
#include "arm_math.h"
#include "stmlib/atan.h"
#include "../../../ChowDSP/include/math_approx/src/trig_approx.hpp"

#define PI_2             1.5707963267948966192313f

using namespace daisy;
using namespace daisysp;

DaisyPod hw;
bool test, print, overrun_chk, use_knobs;



uint32_t mag_time;
uint32_t cmsis_mag_time;
uint32_t stm_mag_time;
uint32_t atan2_time;
uint32_t approx_atan2_time1;
uint32_t approx_atan2_time2;
uint32_t approx_atan2_time3;

uint32_t cos_time, sin_time, cos_time1, sin_time1, cos_time2, sin_time2;
float a, b, x, y, e, f, g, h, i, j, k, l, m, n, o;

Parameter x_k, y_k;

float randomFloat()
{
    return (float)((rand() % 10000) -5000) / (float)(5000);
}

float ApproxAtan2(float y, float x)
{
    const float n1 = 0.97239411f;
    const float n2 = -0.19194795f;    
    float result = 0.0f;
    if (x != 0.0f)
    {
        const union { float flVal; uint32_t nVal; } tYSign = { y };
        const union { float flVal; uint32_t nVal; } tXSign = { x };
        if (fabsf(x) >= fabsf(y))
        {
            union { float flVal; uint32_t nVal; } tOffset = { PI };
            // Add or subtract PI based on y's sign.
            tOffset.nVal |= tYSign.nVal & 0x80000000u;
            // No offset if x is positive, so multiply by 0 or based on x's sign.
            tOffset.nVal *= tXSign.nVal >> 31;
            result = tOffset.flVal;
            const float z = y / x;
            result += (n1 + n2 * z * z) * z;
        }
        else // Use atan(y/x) = pi/2 - atan(x/y) if |y/x| > 1.
        {
            union { float flVal; uint32_t nVal; } tOffset = { PI_2 };
            // Add or subtract PI/2 based on y's sign.
            tOffset.nVal |= tYSign.nVal & 0x80000000u;            
            result = tOffset.flVal;
            const float z = x / y;
            result -= (n1 + n2 * z * z) * z;            
        }
    }
    else if (y > 0.0f)
    {
        result = PI_2;
    }
    else if (y < 0.0f)
    {
        result = -PI_2;
    }
    return result;
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

void MathsCheck() {

	int temp;

	if (use_knobs) {
		x = x_k.Process();
		y = y_k.Process();
	} else {
		x = randomFloat();
		y = randomFloat();
	}

	float mag_in[2];
	mag_in[0] = x;
	mag_in[1]= y;

	// - - - - - - - - MAG - - - - - - - - 
	//Measure - start
	DWT->CYCCNT = 0;
	a = sqrt(x * x + y * y);
	//Measure - end
	mag_time = DWT->CYCCNT;

	temp = DWT->CYCCNT;
	arm_cmplx_mag_f32(mag_in, &b, 1);
	//Measure - end
	cmsis_mag_time = DWT->CYCCNT - temp;
	//Measure - start
	temp = DWT->CYCCNT;
	i = stmlib::fast_rsqrt_accurate(x * x + y * y);
	//Measure - end
	stm_mag_time = DWT->CYCCNT - temp;

	// - - - - - - - - ATAN2 - - - - - - - - 
	//Measure - start
	temp = DWT->CYCCNT;
	e = atan2f(y,x);
	//Measure - end
	atan2_time = DWT->CYCCNT - temp;

	//Measure - start
	temp = DWT->CYCCNT;
	f = ApproxAtan2(y,x);
	//Measure - end
	approx_atan2_time1 = DWT->CYCCNT - temp;
	
	//Measure - start
	temp = DWT->CYCCNT;
	g = atan2_approx(y,x);
	//Measure - end
	approx_atan2_time2 = DWT->CYCCNT - temp;

	//Measure - start
	temp = DWT->CYCCNT;
	h = stmlib::fast_atan2(y, x);
	h = h/10000.0f;
	//Measure - end
	approx_atan2_time3 = DWT->CYCCNT - temp;

	//Measure - start
	temp = DWT->CYCCNT;
	j = cosf(e);
	//Measure - end
	cos_time = DWT->CYCCNT - temp;

	//Measure - start
	temp = DWT->CYCCNT;
	l = arm_cos_f32(e);
	//Measure - end
	cos_time1 = DWT->CYCCNT - temp;

	//Measure - start
	temp = DWT->CYCCNT;
	n = math_approx::cos_mpi_pi<5,float>(e);
	//Measure - end
	cos_time2 = DWT->CYCCNT - temp;
	
	//Measure - start
	temp = DWT->CYCCNT;
	k = sinf(e);
	//Measure - end
	sin_time = DWT->CYCCNT - temp;

	//Measure - start
	temp = DWT->CYCCNT;
	m = arm_sin_f32(e);
	//Measure - end
	sin_time1 = DWT->CYCCNT - temp;

	//Measure - start
	temp = DWT->CYCCNT;
	o = math_approx::sin_mpi_pi<5,float>(e);
	//Measure - end
	sin_time2 = DWT->CYCCNT - temp;

	if (test == false) {
		overrun_chk = true;
	} else {
		overrun_chk = false;
	}

	print = true;

}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	use_knobs = (hw.button1.RisingEdge()) ? !use_knobs : use_knobs;
	test = (hw.button2.RisingEdge()) ? true : false;

	//LEDs
	if(use_knobs)
		hw.led1.Set(0.0f, 1.0f, 0.0f); // green
	else 
		hw.led1.Set(0.0f, 0.0f, 0.0f); // off

	hw.UpdateLeds();

	if (test) {
		MathsCheck();
	}

	for (size_t i = 0; i < size; i++)
	{
		out[0][i] = in[0][i];
		out[1][i] = in[1][i];
	}
}


void printResults(){

	if (overrun_chk) {
		hw.seed.PrintLine("!!!! OVERRUN ALERT !!!!");
	} else {
	
		hw.seed.PrintLine("* * * * * * * * * * * * * * * * * * * * * * * *");
		hw.seed.PrintLine("Magnitude(y,x) \ty = %f, \tx = %f", y, x);
		hw.seed.PrintLine("C++ Standard: \tOutput = %f \tRuntime = %d", a, mag_time);
		hw.seed.PrintLine("CMSIS Mag: \tOutput = %f \tRuntime = %d", b, cmsis_mag_time);
		hw.seed.PrintLine("STM lib Mag: \tOutput = %f \tRuntime = %d", i, stm_mag_time);

		hw.seed.PrintLine("* * * * * * * * * * * * * * * * * * * * * * * *");
		hw.seed.PrintLine("ArcTan2(y,x) \ty = %f, \tx = %f", y, x);
		hw.seed.PrintLine("C++ atan2f: \tOutput = %f, \tRuntime = %d" , e, atan2_time);
		hw.seed.PrintLine("1st approx: \tOutput = %f, \tRuntime = %d" , f, approx_atan2_time1);
		hw.seed.PrintLine("2nd approx: \tOutput = %f, \tRuntime = %d" , g, approx_atan2_time2);
		hw.seed.PrintLine("STM lib: \tOutput = %f, \tRuntime = %d" , h, approx_atan2_time3);

		hw.seed.PrintLine("* * * * * * * * * * * * * * * * * * * * * * * *");
		hw.seed.PrintLine("Cos(e) \te = %f", e);
		hw.seed.PrintLine("C++ Standard: \tOutput = %f \tRuntime = %d", j, cos_time);
		hw.seed.PrintLine("Cmsis: \t\tOutput = %f \tRuntime = %d", l, cos_time1);
		hw.seed.PrintLine("Chow: \t\tOutput = %f \tRuntime = %d", n, cos_time2);
		hw.seed.PrintLine("Sin(e) \te = %f", e);
		hw.seed.PrintLine("C++ Standard: \tOutput = %f \tRuntime = %d", k, sin_time);
		hw.seed.PrintLine("Cmsis: \t\tOutput = %f \tRuntime = %d", m, sin_time1);
		hw.seed.PrintLine("Chow: \t\tOutput = %f \tRuntime = %d", o, sin_time2);
		hw.seed.PrintLine("* * * * * * * * * * * * * * * * * * * * * * * *");
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(96); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_16KHZ);
	hw.StartAdc();

	// Enable Logging, and set up the USB connection.
	hw.seed.StartLog(true);

	// setup measurement
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->LAR = 0xC5ACCE55;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	//Parameters 
	x_k.Init(hw.knob1, -1.0f, 1.0f, Parameter::LINEAR);
	y_k.Init(hw.knob2, -1.0f, 1.0f, Parameter::LINEAR);

	print = false;
	use_knobs = false;

	hw.StartAudio(AudioCallback);
	while(1) {

		if (print) {
			printResults();
			print = false;
		}

		System::Delay(1000);

	}
}
