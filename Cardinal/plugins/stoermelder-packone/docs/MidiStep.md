# stoermelder MIDI-STEP

MIDI-STEP is a module intended for relative MIDI protocols of endless rotary knobs found on some hardware MIDI controllers. The module outputs triggers on INC or DEC for every knob-twisting.

![MIDI-STEP intro](./MidiStep-intro.png)

The module is similiar to VCV MIDI-CC module and receives up to 8 different MIDI CC numbers in default mode and up to 16 different MIDI CC numbers in polyphonic mode. Every MIDI CC message is interpreted according to the selected hardware protocol and one or more short triggers are sent on the INC-port (increment, increased value) or DEC-port (decrement, decreased value).

![MIDI-STEP operating](./MidiStep-cc.gif)

In polyphonic mode (added in v1.6.0) all triggers are sent on the first ports (top left) of the INC- and DEC-sections using a polyphonic connection. This mode especially useful together with [POLY-PILE](./Pile.md) which converts up to 16 channels into absolute voltages.

![MIDI-STEP polyphonic](./MidiStep-poly.png)

## Tested hardware controllers

I don't own all of following devices so I can provide only limited supported for setup and correct function. They have been tested successfully by users though. Feel free to contact me if you have a MIDI controller with endless rotary knobs which do not work with one of the existing relative protocols.

- **Native Instruments controllers.** You have to change the assigned "Control Change" mode to "Relative" using the "Controller Editor" software.

![MIDI-STEP Native Instruments controller](./MidiStep-NI.jpg)

- **Arturia Beatstep and Beatstep Pro.** These controllers support three different relative protocols which can be configured in the "MIDI control center" software. Currently only "Relative #1" and "Relative #2" are supported. #1 should you give more precision when rotating knobs at different speeds.

![MIDI-STEP Arturia controller](./MidiStep-Arturia.jpg)

- **Behringer X-TOUCH Mini.** The controller supports three different relative protocols which can be configured with the "X-TOUCH Editor" software. Currently only "Relative1" is supported.

![MIDI-STEP Behringer controller](./MidiStep-Behringer.jpg)

- **DJ Tech Tools Midi Fighter Twister.** Set the "Encoder Type" to "ENC 3FH/41H" in the MidiFighter Utility.

![MIDI-STEP DJ Tech Tools controller](./MidiStep-DJTT.png)

<a name="akai-mpd218"></a>
- **Akai MPD218.** Set the knob's "TYPE" to "INC/DEC 2" in the MPD218 Editor.

![MIDI-STEP Akai MPD218 Editor](./MidiStep-Akai.png)

MIDI-STEP was added in v1.5 of PackOne.