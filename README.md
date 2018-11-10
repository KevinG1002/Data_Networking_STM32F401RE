# Data_Networking_STM32F401RE
An interesting project working with the STM32F401RE Microcontroller. 
Involves: 
- Reading and writing data-packets to different areas in memory. 
- Reading and Writing to External hardware, EEPROM in this case.
- Performing transmission reliability checks.
- Using the I2C transmission protocol.
- Implementing ACK-polling.

The data-packet that is going to be transmitted is a 60-byte array.

This file contains the following functionalities: 
- Press on the Joystick: Sample Temperature, store it in Memory, then perform a CRC calculation on the data sampled. 
- Press on the Joystick-Right position: Write Temperature to external hardware (EEPROM in this case). 
- Press on the Joystick-Left position: Read Temperature from EEPROM and display it on the LCD screen. 
- Press on the Joystick-Down position (number of times pressed): 
(1) Display next field in packet array. (Should start with MAC-ADDR)
(2) Display next field in packet array.(Should display MAC SRC)
(3) Display next field in packet array.(Should display Length) 
(4) Last field in the packet array. (Should display Payload)
(5) Performs CRC Check on the packet data. (Should show if the CRC Check is successful or not)
Then returns to last field of data-packet. 
- Press on Joystick-Up position (number of times pressed): 
