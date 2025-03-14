// Tracks the peaks in the signal stream using the attack and release parameters

template <int Channels=2, typename Value=float>
class EnvelopeFollower
{
public:
  EnvelopeFollower()
  {
    for (int i = 0; i < Channels; i++)
      m_env[i]=0;
  }

  Value operator[] (int channel) const
  {
    return m_env[channel];
  }

  void Setup (int sampleRate, double attackMs, double releaseMs)
  {
    m_a = pow (0.01, 1.0 / (attackMs * sampleRate * 0.001));
    m_r = pow (0.01, 1.0 / (releaseMs * sampleRate * 0.001));
  }

  void Process (size_t samples, const Value** src)
  {
    for( int i=0; i<Channels; i++ )
    {
      const Value* cur = src[i];

      double e = m_env[i];
      for (int n = samples; n; n--)
      {
        double v = std::abs(*cur++);
        if (v > e)
          e = m_a * (e - v) + v;
        else
          e = m_r * (e - v) + v;
      }
      m_env[i]=e;
    }
  }

  double m_env[Channels];

protected:
  double m_a;
  double m_r;
};