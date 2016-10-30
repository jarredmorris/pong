# Simple Pong Clone
This software drives a simple clone of the game Pong to run on an embedded system using an ARM board. Note that this is a simpler version than the finished version, and does not include features such as displaying user scores, as with other items on my GitHub, this is for plagiarism protection as this assignment may be set to pupils at the university once again. 

It hosts the game for two players; each controlling a bat with an analogue paddle-like joystick and it draws its graphics to an oscilloscope display. This requires a hefty amount of DAC and ADC work. It also relied on an understanding of the physical limitations of moving an electron gun manually to create graphics, as precise movement was needed to prevent a faded appearance or skewed imagery.

Given that this is is a task in embedded systems engineering it is unlikely to compile or run properly on ordinary systems. Please do get in contact if you would like a description of the hardware it was created for. This work naturally required plenty of "talking" to the hardware through registers and was a fascinating exercise in computer organisation and architecture.

No Math library was provided on the system so concepts like random generation had to be implemented by myself.
