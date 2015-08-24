NAME = irdecoder
MCU = atmega328p
FCPU = 20000000

DRIVER = usbtiny

CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS += -Wall -Os -mmcu=$(MCU) -DF_CPU=$(FCPU)

SOURCE = irdecoder.c nec.c usart.c

all:
	avr-gcc $(CFLAGS) -o $(NAME).elf $(SOURCE)
	avr-objcopy -O ihex $(NAME).elf $(NAME).hex
	
flash:	
	sudo avrdude -c $(DRIVER) -p $(MCU) -U flash:w:$(NAME).hex

fuses:
	sudo avrdude -c $(DRIVER) -p $(MCU) -U lfuse:w:0xde:m -U hfuse:w:0xd9:m

clean:
	rm *.elf
	rm *.hex
 
