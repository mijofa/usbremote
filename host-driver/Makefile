CC=gcc
CFLAGS += -Wall -lusb-1.0

# There's already implicit rules for how to build something from a .c, this is just to set the dependencies
usbremoted: device.c commands.c

.DEFAULT_GOAL := usbremoted

clean:
	rm usbremoted

