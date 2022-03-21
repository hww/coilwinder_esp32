# DIY Coil Winder on ESP32

The coil winder based on esp32 and two step motors

1. There is another project based on GRBLhttps://github.com/hww/coil_winder_grbl_esp32
2. This project does not require a computer to wind the coil and has more manual control over process.

# ESP GPIOS

There are two step motors X and R and quad encoder with two additional buttons A,B

**Table 1: The input GPIO pins**

| Function | GPIO |
|----------|------|
| QUAD A   |   16 |
| QUAD B   |   17 |
| QUAD BUTTON | 32 | 
| A BUTTON |   33 | 
| B BUTTON |   15 |
| X HOME POSITION | 35 |

**Table 2: The output gpio pins**

| Function | GPIO |
|----------|------|
| DIR X    |   26 |
| STEP X   |   25 |
| ENA X    |   13 |
| DIR R    |   14 |
| STEP R   |   27 |
| ENA R    |   4 |

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

The display is the menu until we start the winding.
While we are wining the display shows the stats on the screen.
Quad button can enable the menu. To disable we have to use menu
option or one other A or B buttons.

** Table 1: The functions when the winding process started
|     | Not Winding         | Winding       | Completing                |
|-----|---------------------|---------------|---------------------------|
|  Ab | Released            | Hold          | (Click) One turn          |
|  Bb |                     |               | (Click) Next layer        |
| Q-+ | Unwind -1 turn      | *ControlSpeed | Wind + 1 turn             |
|  Qb | Menu                |               | Menu                      |


