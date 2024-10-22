EnigmaCurry's VCV pack
==============================

This is my collection of modules for [VCV Rack](https://vcvrack.com/):

 * [Transport](#Transport)
 * [Latch](#Latch)
 * [Pulse](#Pulse)
 * [Range](#Range)

[These modules are freely available in the VCV Rack
Library.](https://library.vcvrack.com/?brand=EnigmaCurry)

## License

This software is fully licensed under the [GNU General Public License
v3.0+](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/blob/v2/LICENSE),
in the spirit of freely sharing with the VCV Rack and Cardinal
community. At your discretion, you may alternatively use this software
under the terms of the [MIT
license](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/blob/v2/LICENSE.MIT).

The panel artwork incorporates the public domain [Camels Caravan
Silhouette](https://openclipart.org/detail/242201/camels-caravan-silhouette)
([CC0-1.0](https://creativecommons.org/publicdomain/zero/1.0/)). If
you publish your own fork, please use different artwork to help
differentiate the models.

Typeface fonts included are licensed under the SIL OPEN FONT LICENSE Version 1.1:

  * [Fantasque](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/blob/v2/res/fonts/Fantasque/LICENSE.txt) Copyright (c) 2013-2017, Jany Belluz (jany.belluz@hotmail.fr)
  * [DSEG](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/blob/v2/res/fonts/dseg/DSEG-LICENSE.txt) Copyright (c) 2017, keshikan (http://www.keshikan.net)
  * [Manrope](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/blob/v2/res/fonts/manrope/LICENSE.txt) Copyright [https://manropefont.com/](https://manropefont.com/)

## Changelog

 * [CHANGELOG.txt](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/blob/v2/CHANGELOG.txt).
 * [VCV Rack Library updates](https://github.com/VCVRack/library/issues/741).

## Modules

### Transport

You can watch this 20 minute video introduction to Transport and
Latch, or you can read everything below and then download the pre-made
patches:

[![Transport and Latch video](screenshots/Transport%20and%20Latch%20Video.png)](https://odysee.com/@EnigmaCurry:99/transport-and-latch:0)

Transport is a DAW-style play/stop/record control, with clocked
punch-in/punch-out (`Quantize Arming`), allowing you to play and/or
record anything for an exact number of clock cycles (bars). Transport
can control an external recording device to create perfect loops (eg.
with [Lilac Looper](https://library.vcvrack.com/LilacLoop/Looper)).

Transport works both in standalone VCV Rack, or as a VST (VCV Rack Pro
/ Cardinal). Transport can be controlled with your host DAW transport
buttons (play/stop) or other external MIDI transport via MIDI-CV, or
you can use the on-device trigger buttons for `PLAY`, `ARM`, and
`RESET`.

![Transport](screenshots/Transport.png)

 * Set `LENGTH` to the number of clock cycles (eg. beats, bars) that
   you wish to record. (Or toggle the button next to it OFF, to
   disable it, and to record an unlimited length.)
 * Input a clock signal (`CLK`) to count the number of cycles or bars
   elapsed. (Transport has an internal clock divider in the right
   click context menu, which defaults to `4` because there are
   [usually] four beats in one bar. Choose `24PPQN` if you are using
   an external MIDI-CV CLK source.) If you are using [Impromptu
   CLOCKED](https://library.vcvrack.com/ImpromptuModular/Clocked-Clkd)
   as your clock source, make sure to turn OFF `Outputs high on reset
   when not running` in the context menu of CLOCKED. (Your clock
   signal should start low, and then immediately go high when playback
   begins, counting starts at bar `1`. When stopped, the clock should
   return low.)
 * Tap the `ARM` button, or input a trigger from another source (eg.
   STROKE), to arm recording on next play (or to start recording when
   already playing.) If you want to do many recording takes, you can
   wire this from the MIDI-CV `STOP` output, and it will automatically
   re-arm when stopped. When already playing, the `ARM` input is
   quantized according to your selected `Quantize Arming` in the right
   click context menu. This will let you record exactly on the down
   beat (clock high) of the next bar x1, x2, x4 etc. or you can turn
   the quantization `OFF` and record without any delay.
 * Tap the `PLAY` button, or input a trigger from another source (eg.
   STROKE), to start playing. If recording is armed, recording will
   start too. Choose whether you want to have `PLAY` be a toggle
   button, or to behave as a latch (See `Play button is toggle?` in
   the right click context menu). You can wire this from the MIDI-CV
   `START` output so that it starts when your DAW Host transport
   starts.
 * Tap the `RESET` button, or input a trigger from another source, to
   reset, stop playing, and un-arm recording. You can wire this from
   the MIDI-CV `STOP` output and so it would stop when your DAW Host
   transport stops. By default, a `RST` trigger is output everytime
   playback starts or stops. This can be configured in the right click
   context menu under `On Start` and `On Stop`.
 * Output a reset signal (`RST`) from Transport back to your clock
   generator (optional).
 * `PGAT` is output high for the entire time playing. `PTRG` triggers
   a pulse on start and stop.
 * `RGAT` is output high for the entire time recording. `RTRG`
   triggers a pulse on start and stop.
 * Output `PGAT` or `PTRG` to your Clock Generator's `RUN` input.
   (`PGAT` is preferred. For Impromptu Modular's CLOCKED module, set
   `Run CV input is level sensitive` in the right click dialog.)
 * Connect `RGAT` to the recorder GATE input, or `RTRG` to its TRIGGER
   input.
 * The `LOOP` output is a trigger that pulses at the beginning of
   every loop of the record length (whether recording or not). You can
   use this to trigger a sampler so that it synchronizes back to the
   original clock that it was recorded with.
 * In the right click context menu of Transport, set `Quantize Arming`
   if you want the recording to start exactly on the next beat/bar
   x1,2,4,8,16 etc. The arm button will flash while waiting for the
   next quantized beat in order to start or stop recording.
 * In the right click context menu, set `Stop on record length` if you
   wish to stop playback after the recording `LENGTH` counter is
   reached. (When off, the `LENGTH` only affects the recording time.)

Here is an example patch that shows Transport hooked up to the host
DAW transport via MIDI-CV and set to automatically re-arm the
recording on stop for multiple recording takes. The host DAW is set
for 120BPM, Transport's internal clock divider is set to `24 PPQN`,
and the Transport record length is set to 4 bars, so therefore, when
its (automatically) armed and you press play, it will send a trigger
to start recording, and then stop it after exactly 8 seconds as shown
elapsed here:

![Transport in your DAW](screenshots/TransportDAWClock.png)

You can download this as a [VCV Rack selection
`.vcvs`](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/raw/v2/patches/Selections/Transport%20DAW%20Clock.vcvs)

Here is an example patch that acts as a Live Looper platform (see
notes inside the patch):

![Transport Looper](screenshots/TransportLooper.png)

You can download this as a VCV Rack selection
`.vcvs` in several versions: 1) [DAW-less where Transport has primary control](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/raw/v2/patches/Selections/Transport%20Looper.vcvs) and 2) [Controlled by MIDI-CV for use in a DAW.](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/raw/v2/patches/Selections/Transport%20Looper%20MIDI-CV.vcvs) and 3) [MIDI-CV 8 stereo channel version](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/raw/v2/patches/Selections/Transport%20Looper%20MIDI-CV%208ch%20stereo.vcvs)

#### Tool included for splitting a multi-channel .wav from Lilac Looper

In this repository is an included tool called
[wavsplit.sh](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/blob/v2/tool/wavsplit.sh),
which is a BASH script that runs the
[sox](https://sox.sourceforge.net) tool to split a 16-channel .wav
file into 8 separate stereo .wav files. 

 * You will need a copy of [sox](https://sox.sourceforge.net)
   installed (available in any Linux package manager, and probably
   available on Mac and Windows too)
 * The BASH shell. (standard on Linux, and should be on Mac too. For
   Windows try WSL.)
 * Run the script like this: `./wavsplit.sh recording.wav`
 * The script will check to see if all dependencies are met, and then
   attempt to split the `recording.wav` into 8 separate stereo tracks
   named `recording_1.wav` -> `recording_8.wav` in the same directory
   as `recording.wav`.

### Latch

Latch contains two (2) polyphonic CV latching gates ('Flip-flops')
with discrete triggers and reset inputs. A trigger on the `TRIG` input
will set `LATCH` output to +10v. A trigger on the `RESET` input will
set `LATCH` output to 0v. The inputs and outputs are duplicated as two
separate sections, an upper and lower. Each input and output accepts
up to 16 channel polyphonic CV, combining both sections offers up to
32 independent flip-flop gates.

![Latch](screenshots/Latch.png)

Here is a simple example, where Latch transforms any pair of triggers
into a persistent ON/OFF gate:

![Latch ON/OFF](screenshots/LatchOnOff.png)

Here is another example problem that Latch helpfully solves:

[Impromptu Modular's CLOCKED/CLKD
modules](https://library.vcvrack.com/ImpromptuModular/Clocked-Clkd)
have a `RUN` output that only outputs a trigger when the clock is
started AND/OR when stopped (a toggle). The `RUN` input may be
configured to accept a gate signal by choosing the right click context
menu labeled: `Run input CV is level sensitive`, however the `RUN`
output can only be set as a trigger. If you would rather have a single
persistent gate that is either high when playing or low when stopped,
and you also want to have two discrete play and stop buttons/CV inputs
(not a single toggle button), then you can use Latch to solve this:

![Latch Patch](screenshots/LatchPatch.png)

You can download this as a [VCV Rack selection
`.vcvs`](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/raw/v2/patches/Selections/Latch%20CLKD.vcvs)

 * Connect a trigger button to the first `TRIG` input of Latch.
 * Connect a different trigger button to the `RESET` input of Latch.
   In parallel, connect this same trigger button to the `RESET` input
   of CLOCKED.
 * Connect the `LATCH` output of Latch to the `RUN` input of CLOCKED.
   Right click CLOCKED and select `Run input CV is level sensitive`
   and turn OFF `Outputs high on reset when not running.` The same
   `LATCH` output can be used as a "Play mode gate" elsewhere in your
   patch (attached to the scope for example).
 * Press the first trigger button to start the clock. Additional
   presses will not do anything if the latch is already high.
 * Press the second trigger button to stop/reset the clock. Additional
   presses will not do anything to Latch if the latch is already low
   (but will still reset CLOCKED again because its connected
   directly).

The example above only uses the top section of Latch, and only with a
single monophonic Latch output. You may connect up to 16 polyphonic
trigger+reset inputs and 16 latch outputs per section, for a total of
32 latches if you use both sections. Use the Merge and Split (VCV
Fundamental) devices to use polyphony.

Here is a demonstration that uses 20 latches with discrete TRIG and
RESET inputs from Pulses:

![Latch Polyphonic Demonstration](screenshots/LatchPolyphonicDemonstration.png)

You can download this as a [VCV Rack selection
`.vcvs`](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/raw/v2/patches/Selections/Latch%20Demonstration.vcvs)


### Pulse

Pulse is a bank of 16 pulse generators: they turn triggers into gates
with a single shared clocked length. Using 16 channel polyphony, Pulse
has 16 `TRIG` inputs, 16 `GATE` outputs, and 16 `END` outputs. The
triggers are quantized to the incoming clock with configurable
resolution. The `GATE` outputs are held high for the duration of each
independent pulse. The rising and falling edge of the gates are
triggered on discrete `RISE/FALL` jacks. `END` outputs are triggered
on the clock cycle immediately preceding the end of each gate. A
single `LENGTH` parameter controls the length (1 -> 128) of each gate.
All of the gates must share the same length, however they are
triggered and held independently of each other. Use the external
`CLOCK` signal, and the built-in clock divider, and pulse will count
the exact amount of time to keep each gate open at different times. A
shared `RESET` input will immediately reset all of the gates and the
clock.

![Pulse](screenshots/Pulse.png)

Pulse is kind of like a smaller version of [Transport](#Transport),
but with the addition of 16 independent triggers and gates. It can
count clock cycles, like bars of music, or individual beats. It holds
the `GATE` outputs high for the duration you choose. For example, you
can use it as the sustain input to an envelope generator, or as a gate
input on a sampler. (The pulse itself is always a square wave). The
gate interval is configured by the shared `LENGTH` parameter knob. It
has an internal clock divider to configure any common clock
resolution.

The display on Pulse has three rows:

 1. The length parameter used for all of the gates (in red).
 2. The current progress of the top most gate being held. If there is
    only one gate active (on any output) the display is yellow; If
    there is more than one gate active, the display is pink (but only
    shows one counter displaying the lowest numbered active channel).
 3. The clock counter / clock divider (in white). (Only visisble when
    the clock divider is greater than 1.)

The `END` output triggers at the start of the final clock cycle of the
pulse gate, to signal that the pulse is scheduled to end on the next
clock cycle. You can daisy chain several Pulse modules together, to
fire sequentially, by wiring the `END` output of one into the `TRIG`
input of another (each Pulse module must share the same clock source,
but each can have different lengths). If you were to self-patch the
`END` output back into the `TRIG` input of the same Pulse module, the
pulse gate would loop forever.

In the menu, the triggers can be quantized to the next bar, or x2, x4,
x8 etc. With quantization turned on, incoming trigs will wait to start
the gate on the clock. With quantization turned `OFF`, triggers will
fire and hold gates immediately, even before the next clock phase, and
then hold normally for the entire gate length in addition to that.
Thus, with quantization turned `OFF`, the gate will always be a little
bit longer than the length set. For perfect divisions of your clock,
always use quantization.

You can configure what to do on a re-trigger event for a gate that has
already been triggered and with gate already in progress: there are
three choices for the `On Re-trigger during gate` option in the menu:

 * `New Trigger` - Continue the gate and reset the count to 1.
 * `No New Trigger` - Ignore the new trigger, keeping the original count.
 * `Reset` - Ignore the new trigger, but reset the existing gate on
   the next clock phase. (This turns each trig input into a toggle
   switch with clocked time resolution.)

Here is a demonstration of the independent triggers and gates. Just
like in the example used for the timing of Transport, this example
sets Pulse to a gate of 4 bars. The clock is set for 120 BPM. Assuming
that its setup correctly, the timer circuit will watch the gate length
time and show it as exactly 8 seconds.

![Pulse Polyphonic Demonstration](screenshots/PulsePolyphonicDemonstration.png)

You can download this as a [VCV Rack selection
`.vcvs`](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/raw/v2/patches/Selections/Pulse.vcvs)

![Pulse Polyphonic Demonstration 2](screenshots/PulsePolyphonicDemonstration.2.png)

You can download this as a [VCV Rack selection
`.vcvs`](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/raw/v2/patches/Selections/Pulse.2.vcvs)

### Range

Range is a polyphonic latched voltage selector. Input triggers to
latch the selected voltage, held until the next trigger happens, or
until reset.

![Range](screenshots/Range.png)

Range has two selectable voltage ranges, toggleable via the `RANGE`
button:

 * 0V -> 10V : Use any of the 11 input trigger jacks labeled 0V to
   10V.
 * 1V/octave (0V -> 1V): Use any of the 12 input trigger jacks labeled
   C to B.

A single trigger on `RESET` will reset all the outputs.

In the following demonstration, Range is used to select four pitches
from a chord. It is done in two different ways on the two rows, to
demonstrate both 10v and 1v/octave ranges:

![Range Demonstration](screenshots/RangeDemonstration.png)

You can download this as a [VCV Rack selection
`.vcvs`](https://github.com/EnigmaCurry/EnigmaCurry-vcv-pack/raw/v2/patches/Selections/Range.vcvs)

