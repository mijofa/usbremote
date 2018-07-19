NAME = usbremote
MCU = atmega328p

DRIVER = usbtiny

CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS += -Wall -g -Os -mmcu=$(MCU)

V_USB_PATH = v-usb/usbdrv/
CFLAGS += -I $(V_USB_PATH)
# I feel like this is an ugly hack, but it's needed in order to include usbconfig.h
CFLAGS += -I $(PWD)

SOURCE = usbremote.c nec.c
SOURCE += $(V_USB_PATH)/usbdrv.c $(V_USB_PATH)/usbdrvasm.S $(V_USB_PATH)/oddebug.c

all: $(NAME).elf $(NAME).hex

$(NAME).elf: $(SOURCE)
	$(CC) $(CFLAGS) -o $(NAME).elf $(SOURCE)

$(NAME).hex: usbremote.elf
	$(OBJCOPY) -O ihex $(NAME).elf $(NAME).hex

flash: usbremote.hex
	avrdude -c $(DRIVER) -p $(MCU) -U flash:w:$(NAME).hex

fuses:
	avrdude -c $(DRIVER) -p $(MCU) -U lfuse:w:0xde:m -U hfuse:w:0xd9:m

clean:
	rm *.elf
	rm *.hex
 
