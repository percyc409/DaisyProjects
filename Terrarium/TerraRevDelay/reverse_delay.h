#include "daisysp.h"
#include <stdlib.h>

#define MAX_DELAY static_cast<size_t>(48000 * 3.5f)
#define MIN_DELAY static_cast<size_t>(48000 * 0.05f)
#define REV_BLOCK_SIZE static_cast<size_t>(48000 * 0.3f)
//#define MAX_REV_DELAY static_cast<size_t>(48000 * 4.5f)  // Max delay + 2 * Rev_block_length

namespace daisysp
{

class reverse_delay
{

	public:
		reverse_delay() {}
		~reverse_delay() {}

		DelayLine<float, MAX_DELAY> *revDel;
		DelayLine<float, MAX_DELAY> *del;

		void Init(size_t rev_block_len) {
			this->ResetRead();
			revBlockLength = rev_block_len;		
		}

		void ResetRead() { revBlockPos = 0; }

		void SetDelay(float d) {
			delayTarget = (d > MAX_DELAY) ? MAX_DELAY : (d < MIN_DELAY + REV_BLOCK_SIZE) ? (MIN_DELAY + REV_BLOCK_SIZE) : d;
		}

		void SetFeedback(float f) {
			feedback = (f > 1.0f) ? 1.0f : (f < 0.0f) ? 0.0f : f;
		}

		float ProcessRev(float in) {

			float readDelay = currentDelay - revBlockLength + (float)revBlockPos*2.0f;
			float revread = revDel->Read(readDelay);

			//Apply Gain function
			float percentPos = (float)revBlockPos/(float)revBlockLength;
			float gainFunc = 4.0f*(percentPos)*(1-(percentPos));
			revread = revread * gainFunc;

			revBlockPos = (revBlockPos + 1) % revBlockLength;

			revDel->Write(in);

			float fwdread = this->Process(revread*feedback);

			return fwdread + revread;
		}

		float Process(float in)
		{
			//set delay times
			fonepole(currentDelay, delayTarget, .0002f);
			del->SetDelay(currentDelay);

			float read = del->Read();
			del->Write((feedback * read) + in);

			return read;
		}

	protected:

		float currentDelay;
		float delayTarget;

		float feedback;

		size_t revBlockLength;
		size_t revBlockPos;

};

}