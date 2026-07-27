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

    void Init(DelayLine<float, MAX_DELAY> *d) {
        del = d;
        delayTarget = MAX_DELAY/2;
        currentDelay = MAX_DELAY/2;
        feedback = 0.0f;
    };

    float Process(float in) {
         //set delay times
        fonepole(currentDelay, delayTarget, .0002f);
        del->SetDelay(currentDelay);

        float read = del->Read();
        del->Write((feedback * read) + in);

        return feedback*read;
    };

    inline void SetDelayTime(float t) {delayTarget = (t < MAX_DELAY) ? t : MAX_DELAY;}
    inline void SetFeedback(float f)  {feedback = (f < 0.0f) ? 0.0f : (f < 1.3f) ? f : 1.3f;}

  private:

    DelayLine<float, MAX_DELAY> *del;
    float currentDelay;
    float delayTarget;
    float feedback;
};