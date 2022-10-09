# DIY Coil Winder on ESP32

The coil winder based on esp32 and two step motors

1. There is another project based on [GRBL](https://github.com/hww/coil_winder_grbl_esp32)
2. This project does not require a computer to wind the coil and has more manual control over process.

In my optinion, this project do better job than GRBL based version.

<img src="https://github.com/hww/coilwinder_esp32/blob/main/doc/coil_winder_photo_1.jpg" width="900">

<img src="https://github.com/hww/coil_winder_grbl_esp32/blob/main/doc/oled_display.jpg" width="300">

# Project roadmap

- [x] Display SSD1306 
- [x] Step motors driver
- [x] Input buttons and quad encoder
- [x] The menu system  
- [x] Move to home 
- [x] The orthocyclic round coil winder 
- [ ] Better display support
- [ ] The orthocyclic rect coil winder 
- [ ] The helica round coil winder 
- [ ] The helica rect coil winder 
- [ ] Save/Load settings 

# The GPIO usage

There are two step motors X and R and quad encoder with two additional buttons A,B

**Table 1: The input GPIO pins**

| Function    | GPIO |
|-------------|------|
| QUAD A      |   16 |
| QUAD B      |   17 |
| QUAD BUTTON |   32 | 
| A BUTTON    |   33 | 
| B BUTTON    |   15 |
| X HOME POSITION | 35 |

**Table 2: The output gpio pins**

| Function | GPIO |
|----------|------|
| DIR X    |   26 |
| STEP X   |   25 |
| ENA X    |   13 |
| DIR R    |   14 |
| STEP R   |   27 |
| ENA R    |   4  |

**Table 3: SDCARD pins**

| Function | GPIO |
|----------|------|
| CS       |    5 |
| SCK      |   18 |
| MISO     |   19 |
| MOSI     |   23 |

**Table 4: Display pins**

| Function | GPIO |
|----------|------|
| SCK      |   22 |
| SDA      |   21 |

# GUI

The display is working in the menu mode until the winding process is started.
While the winding run the display shows the stats on the screen. When winding 
option or one other A or B buttons.

The auto winding run while A button is pressed in the other case the winder in 
a pause state. 

- In the autowinding mode the quad encoder change the speed. 
- In the pause mode the quad encoder will wind or unwind one turn.
- In the pause mode the quad button will activate menu mode.

There is the special winding mode "Manual Completing". This mode should help 
more precisly complete the layer and start the next one. With this mode the 
autowinding will stop before the end of layer. And the operator can make one 
wind by clicking the button A. And comple the layer by button B. So it allow 

**Table 1: The functions when the winding is not started**
| Button        | Function                        |
|---------------|---------------------------------|
|  A            | Press with Quad endcoder move X |
|  B            | Press with Quad endcoder move R |

**Table 2: The functions when the winding process started**
| Button        | Not Winding         | Autowinding       | Manual Completing         |
|---------------|---------------------|-------------------|---------------------------|
|  A            | Released            | Hold              | (Click) One turn          |
|  B            |                     |                   | (Click) Next layer        |
| Quad Rotation | Unwind -1 turn      | ControlSpeed      | Wind + 1 turn             |
| Quad Button   | Menu                |                   | Menu                      |


# Coil winding process

https://en.wikipedia.org/wiki/Coil_winding_technology

## Round Helical coil

Not implemented yet

## Rectangular Helical coil

Not implemented yet

## Round Orthocyclic coil

Implemented all three possible types: equal count, first layer less, first layer more

## Rectangular Orthocyclic coil

Not implemented yet



