# Bass_CompressorDrive

## Author

C Hershy

## Description

The one stop Bass rig

Compressor -> Overdrive

Overdrive stage uses a hp filter to only apply overdrive to the higher frequencys

-----------------------------------------------------------------------------
Controls
-----------------------------------------------------------------------------

* Knob1    Threshold
* Knob2    Ratio
* Knob3    Makeup Gain
* Knob4    Overdrive Level
* Knob5    Attack OR Distortion
* Knob6    Release OR Cutoff Freq

* Switch 1 KnobMode : (Switch down: Knobs5+6 control compression settings, Switch up: Knobs5+6 control overdrive settings)
* Switch 2 LEDMode : (Switch down: LED2 shows whether overdrive is active, Switch up: LED2 acts as a compression indicator)
*Switch 3 LP dry : Applies a LP filter to the dry signal with cutoff freq equal to hp cutoff freq (May take this out)

* Footswitch 1: Enable/Bypass Compressor stage
* Footswitch 2: Enable/Bypass Overdrive stage

-----------------------------------------------------------------------------
Notes
-----------------------------------------------------------------------------

* Release may not be needed. Could set static and reassign control
* Overdrive signal is too brittle, need to control the high end of the drive better
* Oversampling?
* Need to work on the LO/HIGH frequency splitter. Perhaps replace Cutoff freq knob with switches
