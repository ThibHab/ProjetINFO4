# unless manual

## color and shape of ports

<img src="https://i.imgur.com/f4z6gdl.png" height="250"/>

- input ports are bright and outputs are dark.

- ports that expect or send **triggers or gates** are **square-shaped**
- **CV** and audio related ports are **circular**

- ports with only a thin line around them are secondary in function, usually for mod inputs or less important outputs

## color of modules

most modules have some accent color, this can usually be changed by clicking the `unless` text on the panel. 

for now, the overall color theme can be changed by editing the `RACK_FOLDER/unlessgames/theme.json` file. I'm working on a proper theme editor module for the future..

# modules

  - [avoider](#avoider)
  - [pre-muter](#pre-muter)
  - [pianoid](#pianoid)
  - [towers](#towers)
  - [táncor](#táncor)
  - [room](#room)
  - [SNAKIA](#SNAKIA)
  - [ATOMS](#ATOMS)
  - [mr chainkov](#mr-chainkov)
  - [piong](#piong)

# avoider

<img src="https://library.vcvrack.com/screenshots/unless_modules/avoider.png" height="350"/>

a note generator based on input chords and forbidden/allowed intervals.

# pre-muter

<img src="https://library.vcvrack.com/screenshots/unless_modules/premuter.png" height="350"/>

a simple utility that should be put right before the main audio output, it mutes and fades up the sound when Rack boots up to avoid occasional audio glitches and the sharp starting of patches that always make sound.

  
# pianoid

<img src="https://library.vcvrack.com/screenshots/unless_modules/pianoid.png" height="350"/>

polyphonic note monitor and controller

made for visualizing notes coming from a midi keyboard, generative sources etc. it can function as a sort of "sample and chord" for people who are comfortable with playing chords on a piano keyboard. 
inputting notes with mouse is also possible.

#### usage

  - connect cv and gate from a MIDI-CV module

  - play notes or chords to see them on the piano

  - drag the left bar vertically to scroll up and down the keyboard

  - drag horizontally to zoom in and out

#### sustain

toggle sustain with the `sus` button

when sustaining, `pianoid` will sample notes that are pressed together and hold them until you play the next chord

sustained notes get saved with the patches (and presets), so if you close a patch, you don't lose what you had sustained, you can also use [8FACE](https://library.vcvrack.com/Stoermelder-P1/EightFace) to buffer different chords and automate progressions etc.

#### no input

if no input is connected, you can play notes with your mouse

enable sustain to input chords.


# táncor

a binary tree shaped gate machine inspired by the [cantor set](https://en.wikipedia.org/wiki/Cantor_set)

<img src="https://library.vcvrack.com/screenshots/unless_modules/cantor.png" height="350"/>

five rows of gates each with a ratio slider that controls how the gates inside that row (and every other row under it) gets spread out along the loop

![](https://i.imgur.com/4RUem1i.mp4)

they can be controlled with a phase signal (a rising sawtooth lfo) through the main inputs

any row that doesn't have a phase connected will copy the one above it  

the secondary inputs are cv controls for each sliders  

the bottom gate outputs send out the gates based on the phase inputs  

the cv outputs send out the relative phase of the rows  

switch between gate selecting and the sliders with the "[]" / "|" button at the bottom

#### gate editing

- click on gates to toggle them
- click + drag to change multiple gates
- double-click to either enable or disable all gates inside that row

each gate can be mapped with MIDI-MAP or CV-MAP etc.

polyphonic inputs into the phase inputs will produce polyphonic gate outputs


# towers

<img src="https://library.vcvrack.com/screenshots/unless_modules/towers.png" height="350"/>

polyphonic constant cv source with wrapping polyphonic inputs and mods

polyphonic inputs and mod inputs will be wrapped around as needed. 

for example if you set it to use 8 sliders, and input a 4-channel poly-cable into the MOD input, the channels from 5 to 8 will get modded by the 1-4 channels from the input.

a handy use of the second row of faders is to use it to set a varying mod amount on the first row.

hold **Alt** to set multiple faders in one mouse drag

**SAMPLE** will set the faders to values from the **IN** port

if no cables are connected it will randomize the bank

change between absolute and relative mode of **SAMPLE** in the context menu. 
absolute copies the **IN** values directly, while relative applies the mod amount.

trigger with the **=** button or a gate signal (when you patch a poly cable into the gate inputs, you can trigger it to copy individual channels)


# SNAKIA

emulative snake game modulator

<img src="https://library.vcvrack.com/screenshots/unless_modules/snake.png" height="350"/>

#### controlling

to run the game, connect a clock signal into the step input (central gate port between the arrows, outlined with green)

play the game with the arrow buttons or the corresponding trigger inputs
you can also control your snake with the keyboard (arrow keys or WASD) when you're cursor is over the module 

you can spawn the food manually by triggering the food input

set the spawned food's position with the x, y inputs (if no cables are connected, the food spawns at a random location)

toggle the walls with the square button

control the resolution of the screen with the width and height knobs

#### outputs

the directional trigger outputs on the right fire if the snake hit the corresponding wall, the cv outputs send out the position along the wall as 0-10V

the central gate port (outlined red) is the death output which fires when it is game over (you can adjust the number of frames it takes for the game to restart from the context menu)

#### positions
there are three CV output pairs each sending a position on the map (x-y coordinates)

- the first is the position of the food
- second is where the snake's head is
- third is where the snake ends

#### eye outputs
this snake has three eyes I suppose. they each have a CV output in the bottom left corner sending how far the nearest obstacle is relative to where the snake is heading currently (left eye, forward, right eye)

#### food output
besides the position out of the food there's a trigger output that fires for the frame when the snake eats the food

#### direction and length
below the food output gate, there's the direction output, showing where the snake is currently moving towards
(up : 0V, right : 2.5V, down : 5V, left : 7.5V)

the last output in the bottom left corner outputs the length of the snake (relative to the size of the map)


# room 

<img src="https://library.vcvrack.com/screenshots/unless_modules/room.png" height="350"/>

a helper for arranging the rack

by dragging the directional button in the center, you can move complete row of modules inside rack

it can also add new rows above or below its own and shift/cycle the position of other rows to bring them into the view without scrolling. 

useful when you have a larger number of modules.


# ATOMS

adjustable rotating octa-switch

<img src="https://library.vcvrack.com/screenshots/unless_modules/atoms.png" height="350"/>

the Bohr model might be inaccurate but it looks funky.

Works as a sequential switch, it channels inputs into outputs, both of which are placed on two rotating layers and connected if they are close enough based on the individual ranges (shown as arcs between the "electrons").

right-click on layers to switch between setting positions or ranges and drag electrons like knobs to adjust them.

the e- and e+ buttons equalize input or output layers, right-clicking adds or removes a snapshot of the position/range configuration which can be selected by the smallest central knob.

unused inputs can be used as mod inputs to modulate positions or ranges. 
to select these you need to assign the mod port to an electron by dragging to it either in range mode or position mode (in position mode there is a thin border on each layer where you can move electrons along)
unused outputs can output things like distance to pair in the 0-10V range. 
you can switch between input or output port with right-clicking their button.


There are four modes which can be selected by clicking letters on the "ATOMS" title.
All modes except Mixer connects only one input to any output.
#### A.ttack, Anti-pop or Audio
This mode applies a short anti-pop fade attack to counter pops when switching between connections.
#### T.rigger 
This mode is primarly for mixing trigger or gate signals, it can produce pops when used for audio. It also treats unplugged inputs as 10V gates for creating a sort of gate sequencer without using inputs and just rotating layers or positions.
#### O
Patch a cv signal here to switch between saved snapshots
#### M.ixer
Mixes any input that is within range and also sets their volume accordingly, this can be used as a circular spatial mixer for example. 
It has the highest cpu usage out of all the modes!
#### S.equencer or Sample
To be used as a sequencer or a sample/hold thing. Simple as Trigger mode but instead of dropping back to zero when a connection is lost outputs stay on the last value that came from an input.

in the right-click menu you can choose to switch to audio-rate pairing which you should only do if you want to modulate positions or layers at audio rate as it uses a lot more cpu.


# mr. chainkov

a semi-random melody sequencer based on the [Markov chain model](https://en.wikipedia.org/wiki/Markov_chain)

<img src="https://library.vcvrack.com/screenshots/unless_modules/markov.png" height="350"/>

you can teach him by playing melodies on a keyboard or midi files etc. he will learn by building up a probability map about what notes follow each other most often. then he can traverse this map on its own, generating new melodies along the way that resemble what he was taught.

<img src="https://gitlab.com/unlessgames/unless_modules/raw/master/manual/mrchainkov_manual.png" height="700"/>

# piong

a simulation of the [classic arcade game](https://en.wikipedia.org/wiki/Pong)

<img src="https://library.vcvrack.com/screenshots/unless_modules/piong.png" height="350"/>

It can be played by turning knobs on the screen, or by sending control voltage signals to its inputs (for example from LFOs or MIDI-CC).

The game’s outputs can be used as signals to sequence, trigger and modulate other stuff in your rack. 

![piong_manual.png](https://gitlab.com/unlessgames/unless_modules/raw/master/manual/piong_manual.png)
