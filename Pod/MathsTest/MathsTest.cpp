#include "daisy_pod.h"
#include "daisysp.h"
#include "core_cm7.h"
#include "arm_math.h"

#define PI               3.1415926535897932384626433832795f
#define PI_2             1.5707963267948966192313f

using namespace daisy;
using namespace daisysp;

DaisyPod hw;
bool test, print, overrun_chk, use_knobs;

uint32_t sqrt_time;
uint32_t atan2_time;
uint32_t approx_atan2_time1;
uint32_t approx_atan2_time2;
float a, b, x, y, e, f, g;

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

	// - - - - - - - - SQRT - - - - - - - - 
	a = fabsf(randomFloat())*5.0f;
	//Measure - start
	DWT->CYCCNT = 0;
	b = sqrt(a);
	//Measure - end
	sqrt_time = DWT->CYCCNT;

	// - - - - - - - - ATAN2 - - - - - - - - 
	if (use_knobs) {
		x = x_k.Process();
		y = y_k.Process();
	} else {
		x = randomFloat();
		y = randomFloat();
	}
	//Measure - start
	DWT->CYCCNT = 0;
	e = atan2f(y,x);
	//Measure - end
	atan2_time = DWT->CYCCNT;

	//Measure - start
	DWT->CYCCNT = 0;
	f = ApproxAtan2(y,x);
	//Measure - end
	approx_atan2_time1 = DWT->CYCCNT;
	
	//Measure - start
	DWT->CYCCNT = 0;
	g = atan2_approx(y,x);
	//Measure - end
	approx_atan2_time2 = DWT->CYCCNT;

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
		hw.seed.PrintLine("b = Sqrt(a)");
		hw.seed.PrintLine("a = %f", a);
		hw.seed.PrintLine("b = %f" , b);
		hw.seed.PrintLine("Sqrt Run time: %d Cycles", sqrt_time);

		hw.seed.PrintLine("* * * * * * * * * * * * * * * * * * * * * * * *");
		hw.seed.PrintLine("ArcTan2(y,x) \ty = %f, \tx = %f", y, x);
		hw.seed.PrintLine("C++ atan2f: \tOutput = %f, \tRuntime = %d" , e, atan2_time);
		hw.seed.PrintLine("1st approx: \tOutput = %f, \tRuntime = %d" , f, approx_atan2_time1);
		hw.seed.PrintLine("2nd approx: \tOutput = %f, \tRuntime = %d" , g, approx_atan2_time2);

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
