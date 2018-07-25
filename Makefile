NAME       = usbremote

CLOCK      = 20000000
#CLOCK      = 20000000  # 20MHz
#CLOCK      = 16000000  # 16MHz
#CLOCK      = 8000000   # 8MHz (matches atmega328p's internal RC oscillator)

MCU        = atmega328p
PROGRAMMER = avrisp
PORT       = /dev/ttyACM0
BAUD       = 19200

CC         = avr-gcc
OBJCOPY    = avr-objcopy

CFLAGS += -Wall -g -Os -mmcu=$(MCU) -DF_CPU=$(CLOCK)
CFLAGS += -I $(V_USB_PATH)
# FIXME: This is just to include usbconfig.h, would that be better off in a "config" directory?
CFLAGS += -I $(PWD)

V_USB_PATH = v-usb/usbdrv/
V_USB = $(V_USB_PATH)/usbdrv.c $(V_USB_PATH)/usbdrvasm.S $(V_USB_PATH)/oddebug.c

# NOTE: I'm not using $(NAME) here because I'm defining the dependencies for usbremote, not whatever NAME is specified on the commandline
# FIXME: How do I make this depend on usbconfig.h without adding it to the build arguments?
usbremote: $(V_USB) usbremote.c nec.c

%.hex: %
	$(OBJCOPY) -O ihex "$<" "$@"

flash: $(NAME).hex
	avrdude -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -p $(MCU) -U flash:w:$<

# FIXME: Document these fuses
#        Pretty sure it's just the atmega328p's defaults, with the clock value set to for a 20mhz external clock
fuses:
	avrdude -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -p $(MCU) -U lfuse:w:0xde:m -U hfuse:w:0xd9:m

clean:
	# I suggest using "git clean -X" instead, this list clean never really gets kept up-to-date, but .gitignore does
	-rm -v $(NAME) $(NAME).hex

.DEFAULT_GOAL := $(NAME).hex
