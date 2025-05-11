# ESP32 Logitech Shifter
Convert a Logitech Driving Force Shifter to be usable with any wheel by connecting it directly to the PC via USB.

## Requirements
- Logitech Driving Force Shifter
- ESP32 (For this script at least S3 or other USB capable)
- Soldering iron

## Soldering to ESP32
The shifter has 2 potentiometers and 1 button.  
I simply cut the wires from the shifters daughter board and soldered them to the ESP32.  
Here's the pins I used (potentiometer pins need ADC):

| Wire color | Function                 | ESP32 pin |
|------------|--------------------------|-----------|
| Brown      | Vertical potentiometer   |         6 |
| Red        | Horizontal potentiometer |         4 |
| Orange     | Button                   |        15 |
| Black      | Power                    |       3v3 |
| Yellow     | Ground                   |       GND |

## Modes
Since there are some mods possible (like e.g. rubber bands to make it always center) I also wanted
to have different modes that can make use of this.  
The modes can be changed by quickly pressing the shifter down twice.

* H-Mode behaves like the standard logitech shifter.
Gears 1-6 are assigned buttons 1-6.
Reverse gear can be activated by pressing down and selecting 6th gear.
Reverse gear is mapped to button 7

* Sequential only differentiates between up and down state.
Left and right movement has no effect.
Gear up is assigned button 8, Gear down is assigned button 9.

* Handbrake mode only checks for the down state.
If the shifter is down, button 10 is pressed.
