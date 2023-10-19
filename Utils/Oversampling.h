#ifndef OVERSAMPLING_H
#define OVERSAMPLING_H

#include <stddef.h>

template<size_t audio_block_size, size_t NUM_CHANS>
class Oversampling
{
    public:
        Oversampling(){};
        ~Oversampling(){};

    auto getArray()
        {
            return oversample_;
        }

    void Init() {
        ClearBuff();
    }

    void Upsample(const float* const* in,  size_t size)
    {
        for (int chan = 0; chan < NUM_CHANS; chan++)
        {
            // 3 buffers - ring buffer for input, fir buffer and output buffer

            // loop through input buffer
            for (size_t i = 0; i < size; i++)
            {
                // Move current input sample to 2nd to last index in ring buffer
                // also does 0 padding
                ring_buffer_[chan][N - 1] = 2 * in[chan][i];

                // Convolution
                float out = 0.;

                for (size_t k = 0; k < Ndiv2; k += 2) // Every other sample
                    out += (ring_buffer_[chan][k] + ring_buffer_[chan][N - k - 1]) * fir[k]; // Convolve 2 samples
                
                // Outputs
                oversample_[chan][i << 1] = out;
                oversample_[chan][(i << 1) + 1] = ring_buffer_[chan][Ndiv2 + 1] * fir[Ndiv2];

                // Shift data in ring buffer
                for (size_t k = 0; k < N -2; k +=2)
                    ring_buffer_[chan][k] = ring_buffer_[chan][k + 2];
            }
        }
    }

    void Downsample(const float* const* in, float** out, size_t size)
    {

        for (int chan = 0; chan < NUM_CHANS; chan++)
        {
            // 3 buffers - ring buffer for input, fir buffer and output buffer
            int pos = position[chan];
            // loop through input buffer
            for (size_t i = 0; i < size; ++i)
            {
                // Go through ever other sample of the oversampled block, move to ring buffer
                ring_buffer2_[chan][N - 1] = oversample_[chan][i << 1];

                // Convolution
                float output = 0.;

                for (size_t k = 0; k < Ndiv2; k += 2) // Every other sample
                    output += (ring_buffer2_[chan][k] + ring_buffer2_[chan][N - k - 1]) * fir[k]; // Convolve 2 samples
                
                // Outputs
                output += ring_buffer3_[chan][pos] * fir[Ndiv2];
                ring_buffer3_[chan][pos] = oversample_[chan][(i << 1) + 1];

                out[chan][i] = output;

                // Shift data in ring buffer
                for (size_t k = 0; k < N -2; k +=2)
                    ring_buffer2_[chan][k] = ring_buffer2_[chan][k + 2];

                pos = (pos == 0? Ndiv4 : pos -1);
            }

            position[chan] = pos;
        }
    };

    void ClearBuff()
    {
        for(int i =0; i<47; i++){
            ring_buffer_[0][i] = 0.0f;
            ring_buffer_[1][i] = 0.0f;
            ring_buffer2_[0][i] = 0.0f;
            ring_buffer2_[1][i] = 0.0f;
            
            if (i < 12){
                ring_buffer3_[0][i] = 0.0f;
                ring_buffer3_[1][i] = 0.0f;
            }
            
        }

    }

    private:
    
    int N = 47;
    int filter_order = N - 1;
    int Ndiv2 = N / 2;
    int Ndiv4 = N / 4;

    float ring_buffer_[2][47];
    float ring_buffer2_[2][47];
    float ring_buffer3_[2][12]; // NDIV4 + 1

    float oversample_[2][audio_block_size * 2];
    int position[2];

    float fir [47] = 
    {-0.000116310803,
	0,
	0.000455625792,
	0,
	-0.001175144454,
	0,
	0.002495617373,
	0,
	-0.004704627208,
	0,
	0.008179944940,
	0,
	-0.013453543186,
	0,
	0.021387722343,
	0,
	-0.033690825105,
	0,
	0.054717078805,
	0,
	-0.100511685014,
	0,
	0.316407203674,
	0.500000000000,
	0.316407203674,
	0,
	-0.100511685014,
	0,
	0.054717078805,
	0,
	-0.033690825105,
	0,
	0.021387722343,
	0,
	-0.013453543186,
	0,
	0.008179944940,
	0,
	-0.004704627208,
	0,
	0.002495617373,
	0,
	-0.001175144454,
	0,
	0.000455625792,
	0,
	-0.000116310803};
};


#endif