# 2.2.7 - bug fix
- fixing integer based knob rendering that was broken on recent Rack versions

# 2.2.6 - bug fixes
- fixing memory leaks thanks to falkTX
- removing a forgotten warn log from pianoid

# 2.2.5 - bug fixes
- fixing a crash with ATOMS when duplicated

# 2.2.4 - bug fixes
- fixing crashing on táncor gate toggling
- making piong have longer trigger outputs

# 2.2.3 - v2 fixes
- pianoid pressed key display fix
- snakia persistent outputs

## 2.2.2 - v2

- support dark room features
- port labels everywhere

## 1.2.1 - snakia fix

- update direction output after death
- fix ports not outputting if they were plugged in mid-game

## 1.2.0 - melody update

#### new modules

- **pianoid**
  - piano-style controller and chord sustainer
- **avoider**
  - note cv corrector based around the "avoid note" concept
- **pre-muter**
  - mutes signals on Rack launch to avoid noise bursts

#### updates 

- towers
  - random sampling without **IN** is now scaled by the **MODS** knobs
  - random sampling is different in absolute and relative mode
  - switching to param based channel setting
  - bipolar param tooltip now displays correct values
  - staff notation
    - show sharps as +
    - fix out of view notes

- start migrating to art files for layout data
- update licensing

## 1.1.1 - towers updates and fixes

- towers
  - switching to **Alt** and click to draw faders with mouse
  - sample values from **IN** port to set faders
  - relative and absolute sampling modes
  - randomize faders in bank when no **IN** is connected
  - alternative fader draw modes (staff notation & omrinsis)

- tancor
  - fix draggable bottom button
  - fix setting gates on cabled hover


## 1.1.0 - 2nd anniversary update

#### new modules
- SNAKIA
- táncor
- towers
- room

#### color theme
- testing a color theme applied to most visual components
- saved and read from _RACK_DIRECTORY/unlessgames/theme.json_ using rgb color values.
- removing dark theme option from Mr.Chainkov as a result, default becomes somewhat dark.