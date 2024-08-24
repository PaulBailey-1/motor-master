# MotorMaster
This is a board designed to easily command FRC motors for testing or prototyping. It currently supports motors with the Talon FX motor controller. This repo contains the board schematics, PCB layout, and firmware for the board. 

## Features
It features 2 PWM channels that can be connected directly to the CAN connectors of the motor to command a duty cycle. Pressing the test button will output a 50% duty cycle on both PWM channels.  

The board can also be connected to motors through the CAN bus. Connect one end of the CAN chain to the CAN connector on the board and the other end to the TERM connector, or use another termination resistor. 
NOTE: CAN functionality is still experimental and may not work, feel free to work on it.

The e-stop button will prevent the board from outputting any commands until the MCU is reset with the reset button. 

The EXT pins can be used to connect an external input, like a switch or button, to provide additional control such as driving a motor or as an e-stop. The functionality must be added to the code, by default it does nothing.

The board can be powered through the USB C port, or from the power terminal off of a 12V battery. The board does not supply the motor with power, but it is important that it shares ground with the motor. 

The board is designed to be used over Bluetooth with its accompanying app, the source of which is here: https://github.com/PaulBailey-1/motor-master-app  
It is a cross-platform app and so should be able to be compiled for Android and iOS, but it has only been tested for Android. 

## Manufacturing
Board gerbers exported for manufacture by OSHPark are included in the repo, otherwise, the board can be opened in KiCad to export for manufacture. The BOM can be ordered from either Mouser or DigiKey.  

The FTDI header (J5) and the EXT header (J6) are optional. The USBC TVS diode (D2) may also be omitted since it is hard to solder without bridges, although it does make the USB lines vulnerable to damage from ESD. The USB C may also be omitted entirely (J1 & D2), and the board can be programmed through the FTDI header. 

To assemble the board, a stencil can be ordered and used to apply solder paste to the pads. The solder then may be reflowed in an oven or with hot air. It may also be possible to have it assembled by the PCB fab house.

## Programming
Dependencies:
* CAN by Sandeep Mistry v0.3.1
* ESP32Servo by Kevin Harrington & John K. Bennett v3.0.5

To program the board, connect to it with USB C or with an FTID adapter and program it with the Arduino IDE. Install the ESP32 boards and the required libraries, select 'ESP32S3 Dev Module' as the board, and upload.  

The first time the board is programmed after being assembled, the Boot button must be held from when it is plugged until it has been flashed. For debugging over USB C make sure the 'USB CDC On Boot' option is enabled so that you can get logging over the serial port.
