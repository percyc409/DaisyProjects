#include "daisysp.h"
#include "daisy_pod.h"

using namespace daisysp;
using namespace daisy;

static DaisyPod pod;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{

    //audio
    for(size_t i = 0; i < size; i += 2)
    {
        // left out
        out[i] = in[i];
        // right out
        out[i + 1] = in[i + 1];

    }
}

int main(void)
{
    // initialize pod hardware and oscillator daisysp module
    float sample_rate;

    //Inits and sample rate
    pod.Init();
    pod.SetAudioBlockSize(4);
    sample_rate = pod.AudioSampleRate();

    /*
    rev.Init(sample_rate);
    dell.Init();
    delr.Init();
    tone.Init(sample_rate);

    //set parameters
    deltime.Init(pod.knob1, sample_rate * .05, MAX_DELAY, deltime.LOGARITHMIC);
    cutoffParam.Init(pod.knob1, 500, 20000, cutoffParam.LOGARITHMIC);
    crushrate.Init(pod.knob2, 1, 50, crushrate.LOGARITHMIC);

    //reverb parameters
    rev.SetLpFreq(18000.0f);
    rev.SetFeedback(0.85f);

    //delay parameters
    currentDelay = delayTarget = sample_rate * 0.75f;
    dell.SetDelay(currentDelay);
    delr.SetDelay(currentDelay);
    */
   
    // start callback
    pod.StartAdc();
    pod.StartAudio(AudioCallback);

    while(1) {}
}