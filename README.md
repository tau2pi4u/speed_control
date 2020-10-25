# Speed control

An arduino project to support a larger circuit for model railway speed control. It takes two, 4 bit inputs to select a profile and a speed, and outputs PWM to produce the appropriate voltage when combined with additional circuitry. 

## Requirements
* Requires python 3.6 or higher
* Requires pandas and numpy library (`python -m pip install -r requirements.txt`)
* Arduino IDE
* Arduino Nano (or equivalent)

## Usage
Fill in `profiles.csv` with up to 16 profiles, each with up to 16 voltages. These should be between 0V and 5V (they will be clamped to this by the generator script) and will set the average voltage of the PWM output (e.g. 0V means a duty cycle of 0, 2.5V a duty cycle of 0.5 and 5V a duty cycle of 1.0). See the example csv for the structure.  
Run profile_gen.py to produce the header file 'speed_control/profiles.h'.  
Compile and upload to an appropriate arduino.  

### Changing pins
To change the pins, modify `speed_control/pin.h`. The pin arrays start with the LSB of the binary input.

### Increasing the number of speeds/profiles
To increase the number of speeds/profiles it is possible to select, add extra pins to the pin header and increase the definition of the number of pins. 

### profiles.csv
* The profiles file must be a csv called `profiles.csv` in the same folder as `profile_gen.py`, which must then contain the `speed_control/` folder for the .ino and other headers
* `profiles.csv` must have a header row of: `profile_id, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9` (spaces optional).
* Profile ids must be consecutive integers starting at 0
* Voltages should be between 0V and 5V - values outside this range will be clamped to it

## Associated Circuity
Hopefully you're finding this because you've come from a page which describes it, but I'll try to get a diagram/link to show how to construct the circuit into which this fits.

## Smooth Stepping
Smooth stepping is enabled by default. To change the transition time, change TRANS_TIME. To set a maximum rate of change, define MAX_STEP and change the value MAX_STEP to change the maximum step per loop and LOOP_DELAY to set the minimum loop time. The maximum rate of change will be:  
`5000mV * (MAX_STEP / 255) / (LOOP_DELAY * 1000 * 1s)`mV/s