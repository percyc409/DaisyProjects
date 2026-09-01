#pragma once

/**
Digital Delay Class for Echo effect
    \param MAX_DELAY: in Samples
*/

using namespace daisy;
using namespace daisysp;

template <size_t MAX_DELAY, size_t CHANNELS=1>
class Dig_Delay
{
  public:

    Dig_Delay() {}
    ~Dig_Delay() {}

    void Init(float SampleRate, DelayLine<float, MAX_DELAY> *d) {
        del = d;
        delayTarget = MAX_DELAY/2;
        currentDelay = MAX_DELAY/2;
        feedback = 0.7f;
        mix = 1.0f;

        FeedbackFilt.Init(SampleRate);
        FeedbackFilt.SetFilterMode(LadderFilter::FilterMode::LP12);
        FeedbackFilt.SetFreq(10000.0f); // 10KHz
    };

    float Process(float in) {
         //set delay times
        fonepole(currentDelay, delayTarget, .0002f);
        del->SetDelay(currentDelay);

        float read = del->Read();
        read = FeedbackFilt.Process(read);
        read = feedback * read;
        del->Write((feedback * read) + in);

        return (in + mix*read);
    };

    inline void SetDelayTime(float t) {delayTarget = (t < MAX_DELAY) ? t : MAX_DELAY;}
    inline void SetFeedback(float f)  {feedback = (f < 0.0f) ? 0.0f : (f < 1.3f) ? f : 1.3f;}
    inline void SetMix (float m) {mix = (m < 0.0f) ? 0.0f : (m < 1.0f) ? m : 1.0f;}
    void SetTone(float f) {
      f = daisysp::fclamp(f, 0.0f, 1.0f);
      FeedbackFilt.SetFreq(800.0f + 15200.0f * f); // Limits ranges to 800Hz - 16KHz
    }


  private:

    DelayLine<float, MAX_DELAY> *del;
    LadderFilter FeedbackFilt;
    float currentDelay;
    float delayTarget;
    float feedback;
    float mix;
};