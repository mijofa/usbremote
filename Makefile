NAME = usbremote
MCU = atmega328p

DRIVER = usbtiny

CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS += -Iusbdrv/ -Wall -g -Os -mmcu=$(MCU)

SOURCE = usbdrv/usbdrv.c usbdrv/usbdrvasm.S usbremote.c nec.c

all: $(NAME).elf $(NAME).hex

$(NAME).elf: $(SOURCE)
	avr-gcc $(CFLAGS) -o $(NAME).elf $(SOURCE)

$(NAME).hex: usbremote.elf
	avr-objcopy -O ihex $(NAME).elf $(NAME).hex

flash: usbremote.hex
	sudo avrdude -c $(DRIVER) -p $(MCU) -U flash:w:$(NAME).hex

fuses:
	sudo avrdude -c $(DRIVER) -p $(MCU) -U lfuse:w:0xde:m -U hfuse:w:0xd9:m

clean:
	rm *.elf
	rm *.hex
 
