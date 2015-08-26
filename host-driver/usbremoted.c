#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <libusb-1.0/libusb.h>
#include "device.h"
#include "commands.h"


/* stop signal, set to 1 when we should exit */
static uint8_t stop;

/* vendor and product ID for usbremote */
static uint16_t vid = 0x16C0;
static uint16_t pid = 0x05DC;

/* signal handler routine to handle interrupts */
void signal_handler(int s)
{
	switch(s) {
		case SIGHUP:
		case SIGSTOP:
		case SIGINT:
			stop = 1;
			printf("caught signal, stopping...\n");
	}
}

int main(int argc, char**argv)
{

	/* install signal handlers */
	signal(SIGHUP, signal_handler);
	signal(SIGSTOP, signal_handler);
	signal(SIGINT, signal_handler);

	/* read the commands map from the file. loads int->string map where int is
	 * the command code and the string is a command to be executed upon receiving
	 * the associated command code */
	char **commands;
	commands = read_code_commands(DEFAULT_COMMANDS_FILE_PATH);
	if(commands == NULL) {
		printf("err: failed to read commands file at %s: errno=%d\n", DEFAULT_COMMANDS_FILE_PATH, errno);
	}

	/* define our start label here, so any time there is an error or device connect
	 * we jmp to here to restart device initialization */
	start:;

	/* initialize libusb context  */
	libusb_context *usb_ctx;
	if(libusb_init(&usb_ctx) != LIBUSB_SUCCESS) {
		printf("err: failed to initialize libusb\n");
		free_commands(commands);
		return -1;
	}

	/* open a handle to the device that has our vendor and product ID */
	libusb_device_handle *device;
	device = open_device(usb_ctx, vid, pid);
	if(device == NULL) {
		printf("err: failed to open device\n");
		free_commands(commands);
		return 0;
	}

	printf("ok: opened device vid=0x%04X pid=0x%04X\n", vid, pid);
	printf("ok: ready\n");

	int result;
	struct usbremote_packet packet;
	while(!stop) {
		/* attempt to read a packet from the device */
		result = read_remote_data(device, &packet, 1);

		/* return value of less than 0 is an error. in this case we assume we
		 * need to attempt to reestablish a connection to the device */
		if(result < 0) {
			printf("error or device disconnect\n");
			close_device(device);
			libusb_exit(usb_ctx);
			goto start;
		}

		/* if the value is nonzero then we have successfully read a packet */
		if(result) {
			/* if the pointer at the index equal to value of the command is
			 * not null then we have a command to be executed for it */
			if(commands[packet.cmd] != NULL) {
				system(commands[packet.cmd]);
			}

			printf("info: addr=0x%02X cmd=0x%02X\n", packet.addr, packet.cmd);
		}
	}

	printf("done\n");
	close_device(device);
	libusb_exit(usb_ctx);
	free_commands(commands);
	return 0;
}
