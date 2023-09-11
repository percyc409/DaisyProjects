#include "daisy_pod.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

//Setting Struct containing parameters we want to save to flash
struct Settings {
	float p1; // DryWet
	float p2; // Feedback

	//Overloading the != operator
	//This is necessary as this operator is used in the PersistentStorage source code
	bool operator!=(const Settings& a) const {
        return !(a.p1==p1 && a.p2==p2);
    }
};

//Persistent Storage Declaration. Using type Settings and passed the devices qspi handle
PersistentStorage<Settings> SavedSettings(hw.seed.qspi);

//Reverb and Parameters
ReverbSc Verb;
float DryWet;
Parameter Feedback;

bool use_preset = false;
bool trigger_save = false;


void Load() {

	//Reference to local copy of settings stored in flash
	Settings &LocalSettings = SavedSettings.GetSettings();
	
	DryWet = LocalSettings.p1;
	Verb.SetFeedback(LocalSettings.p2);

	use_preset = true;
}

void Save() {

	//Reference to local copy of settings stored in flash
	Settings &LocalSettings = SavedSettings.GetSettings();

	LocalSettings.p1 = DryWet;
	LocalSettings.p2 = Feedback.Value();

	trigger_save = true;
}

void ProcessControls() {

	hw.ProcessAllControls();

	//Switches
	if(hw.button1.RisingEdge()){
		if(use_preset)
			use_preset = false;
		else 
			Load();
	}

	if(hw.button2.RisingEdge()){
		Save();
	}

	//Knobs
	if(!use_preset){
		Verb.SetFeedback(Feedback.Process());
		DryWet = hw.knob1.Process();
	}

	//LEDs
	if(use_preset)
		hw.led1.Set(0, 1, 0); // green
	else 
		hw.led1.Set(1, 0, 0); // red

	if(trigger_save)
		hw.led2.Set(0, 1, 0); // green
	else 
		hw.led2.Set(0, 0, 0); // red

	hw.UpdateLeds();

}


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float inl, inr, outl, outr;
	ProcessControls();

	for (size_t i = 0; i < size; i++)
	{
		inl = in[0][i];
		inr = in[1][i];

		Verb.Process(inl, inr, &outl, &outr);
    	out[0][i] = DryWet * outl + (1 - DryWet) * inl;
    	out[1][i] = DryWet * outr + (1 - DryWet) * inr;
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	Verb.Init(hw.AudioSampleRate());
	Feedback.Init(hw.knob2, 0.0f, 1.0f, daisy::Parameter::LINEAR);
	
	//Initilize the PersistentStorage Object with default values.
	//Defaults will be the first values stored in flash when the device is first turned on. They can also be restored at a later date using the RestoreDefaults method
	Settings DefaultSettings = {0.0f, 0.0f};
	SavedSettings.Init(DefaultSettings);

	hw.StartAdc();
	hw.StartAudio(AudioCallback);
	while(1) {
		if(trigger_save) {
			
			SavedSettings.Save(); // Writing locally stored settings to the external flash
			trigger_save = false;
		}
		System::Delay(100);
	}
}
