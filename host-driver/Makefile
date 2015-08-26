NAME=usbremoted
CC= clang
CFLAGS += -Wall -lusb-1.0
SOURCE = usbremoted.c device.c commands.c

all:
	$(CC) $(CFLAGS) -o $(NAME) $(SOURCE)

clean:
	rm $(NAME)
 
