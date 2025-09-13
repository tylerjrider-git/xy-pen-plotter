# xy-pen-plotter
Simple repo containing code to running Raspberry Pi X-Y Pen Plotter

File Overview:

* bgt_tcm2209 -> low level driver of the BGTTCM2209 stepper motor driver. The pins used
  are hardcoded in the bgt_tcm2209, and should be updated accordingly to the boards pinmux/pinout.

* StepperController -> Class to independently step a motor, can use multiple depending on number
 of motors in application. Will eventually want to be wrapped in some "LinearDriver" class that
 can convert the desired X-Y position of the pen to the appropriate location.

 * XboxController -> A small helper class that can be used read a few xbox controller events, useful
  if we want to plumb with the stepper controller to test the motors and directions without hard coding commands.

  * main.cpp -> Simple CLI to drive the bgt_tcm2209, has an interactive mode, but want to deprecate for XboxController/KeyBoard Controller classes (e.g. read in keyboard events vs grabbing from terminal.)