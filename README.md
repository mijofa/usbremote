usbremote
=========

A USB infrared receiver for NEC-compatible remotes, built using an AVR micrcontroller. Includes host software using libusb. The host software is written for linux, but can be easily adapted for any other system.

###Compliation and Flashing

The firmware using avr-libc and is built using the AVR GCC toolchain. Assuming you have these installed, you can build and flash the firmware onto the device using make:

    make 
    make fuses
    make flash

Building the host software uses clang but can be switched to GCC with minimal changes to the Makefile.

    make

The host software looks for a file /etc/usbremote_commands, which is expected to be in the following format:

    commandcode:command
    0:reboot


###Schematic

![usbremote schematic](http://nikharris.com/content/images/2015/08/usbremote-schematic.png)
