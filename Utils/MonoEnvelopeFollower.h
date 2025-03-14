// Tracks the peaks in the signal stream using the attack and release parameters
// Edited because I cant work out how to pass the AudioHandle::InputBuffer type to the process function

template <typename Value=float>
class MonoEnvelopeFollower
{
public:
  MonoEnvelopeFollower()
  {
    m_env=0;
  }

  double Out() 
  {
    return m_env;
  }

  void Setup (int sampleRate, double attackMs, double releaseMs)
  {
    m_a = pow (0.01, 1.0 / (attackMs * sampleRate * 0.001));
    m_r = pow (0.01, 1.0 / (releaseMs * sampleRate * 0.001));
  }

  void Process (Value src)
  {
      Value cur = src;

      double e = m_env;

      double v = std::abs(cur);
      if (v > e)
        e = m_a * (e - v) + v;
      else
        e = m_r * (e - v) + v;

      m_env=e;
  }

  double m_env;

protected:
  double m_a;
  double m_r;
};