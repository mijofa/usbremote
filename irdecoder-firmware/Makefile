NAME = irdecoder
MCU = attiny2313

DRIVER = usbtiny

CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS += -Iusbdrv/ -Wall -g -Os -mmcu=$(MCU)

SOURCE = irdecoder.c

all:
	avr-gcc $(CFLAGS) -o $(NAME).elf $(SOURCE)
	avr-objcopy -O ihex $(NAME).elf $(NAME).hex
	
flash:	
	sudo avrdude -c $(DRIVER) -p $(MCU) -U flash:w:$(NAME).hex

fuses:
	sudo avrdude -c $(DRIVER) -p $(MCU) -U lfuse:w:0xce:m -U hfuse:w:0xdf:m

clean:
	rm *.elf
	rm *.hex
 
