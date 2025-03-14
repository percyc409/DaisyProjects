// Tracks the peaks in the signal stream using the attack and release parameters

template <typename Value=float>
class EnvelopeFollower
{
public:
  EnvelopeFollower()
  {
    m_env=0;
  }

  Value Value() 
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

      double e = m_env[0];

      double v = std::abs(cur);
      if (v > e)
        e = m_a * (e - v) + v;
      else
        e = m_r * (e - v) + v;

      m_env[0]=e;
  }

  double m_env;

protected:
  double m_a;
  double m_r;
};