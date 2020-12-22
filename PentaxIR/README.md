# PentaxIR

This is an automated Pentax IR trigger for taking multiple long exposures. I designed this primarily for use in astrophotography.

### Modes of Operation:
There are currently two modes of operation:

 1. **Bulb** Mode: This is the default mode. Automatically issue a shutter command at the beginning and end of the exposure. Wait 6 seconds and repeat. (See "Long Press" to configure exposure time)
 2. **Repeat** Mode: This mode issues a shutter command every X seconds (See "Long Press" to configure delay). The camera will end the exposure after its configured shutter time.

### Controlling PentaxIR:
The device is controlled by use of a single button. There are a couple different types of button presses

 - Command Presses: These presses are short, less than 1 second in duration. Command presses are used to set the mode of operation.
	- One quick press begins auto triggering the shutter. The red LED will flash once.
	- Two quick presses put the device into **Bulb** mode. The red LED will quickly flash 3 times.
	- Three quick presses put the device into **Repeat** mode. The red LED will quickly flash 5 times.
- Long Press: A button press greater than 1 second will start setting the exposure length or the delay time. While the button is pressed the red LED will blink. Depending on mode, each flash represents:
	- **Bulb** Mode: Each flash represents 1 minute of exposure.
	- **Repeat** Mode: Each flash represents 1 second of time between exposures.

