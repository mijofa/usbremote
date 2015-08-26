#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <libusb-1.0/libusb.h>

#define COMMANDS_FILE_PATH "/etc/usbremote_commands"

static uint8_t stop;

char **read_code_commands(char *path)
{
	int fd;
	fd = open(path, 0);
	if(fd == -1) {
		return NULL;
	}

	struct stat fs;
	if(fstat(fd, &fs) != 0) {
		close(fd);
		return NULL;
	}

	char *contents = malloc(fs.st_size);
	if(read(fd, contents, fs.st_size) != fs.st_size) {
		close(fd);
		free(contents);
		return NULL;
	}

	int commands_size = sizeof(char*) * 255;
	char **commands = (char **)malloc(commands_size);
	memset(commands, 0, commands_size);

	char *token;
	token = strtok(contents, "\n");
	if(token != NULL) {
		do {
			char *sep_offset;
			sep_offset = strstr(token, ":");
			if(sep_offset == NULL) {
				printf("err: failed to parse command in file: %s", token);
				continue;
			}

			int index;
			char *index_str;
			index_str = malloc(sep_offset - token + 1);
			memcpy(index_str, token, sep_offset - token);
			index = atoi(index_str);
			free(index_str);

			commands[index] = strdup(sep_offset + 1);
			printf("DEBUG: loaded command index %d = %s\n", index, commands[index]);

		} while((token = strtok(NULL, "\n")));
	}

	close(fd);
	free(contents);
	return commands;
}

void free_commands(char **commands) {
	int x;
	for(x = 0; x < 255; x++) {
		if(commands[x] != NULL) {
			free(commands[x]);
		}
	}

	free(commands);
}

void signal_handler(int s)
{
	switch(s) {
		case SIGHUP:
		case SIGSTOP:
		case SIGINT:
			stop = 1;
	}
}

libusb_device_handle *open_device(libusb_context *ctx, uint16_t vid, uint16_t pid)
{
	libusb_device_handle *device;
	device = NULL;
	while(!stop) {
		device = libusb_open_device_with_vid_pid(ctx, vid, pid);
		if(device != NULL) {
			printf("DEBUG: connected\n");
			return device;
		}

		sleep(1);
	}

	return NULL;
}

int main(int argc, char**argv)
{
	uint16_t vid = 0x16C0;
	uint16_t pid = 0x05DC;

	signal(SIGHUP, signal_handler);
	signal(SIGSTOP, signal_handler);
	signal(SIGINT, signal_handler);

	int err;

	char **commands;
	commands = read_code_commands(COMMANDS_FILE_PATH);
	if(commands == NULL) {
		printf("err: failed to read commands file at %s: errno=%d\n", COMMANDS_FILE_PATH, errno);
	}

	start:;

	libusb_context *usb_ctx;
	if(libusb_init(&usb_ctx) != LIBUSB_SUCCESS) {
		printf("err: failed to initialize libusb\n");
		return -1;
	}

	libusb_device_handle *device;
	device = open_device(usb_ctx, vid, pid);
	if(device == NULL) {
		printf("done\n");
		return 0;
	}

	printf("ok: opened device vid=0x%04X pid=0x%04X\n", vid, pid);

	err = libusb_set_configuration(device, 1);
	if(err) {
		printf("err: failed to set configuration for device: %s\n", libusb_error_name(err));
		return -1;
	}

	printf("ok: configuration set to 1\n");


	if(libusb_kernel_driver_active(device, 1) == 1) {
		printf("info: kernel driver active, detaching\n");
		if((err = libusb_detach_kernel_driver(device, 0)) != LIBUSB_SUCCESS) {
			printf("err: failed to detach kernel driver: %s", libusb_error_name(err));
			libusb_close(device);
			libusb_exit(usb_ctx);
			return -1;
		}

		printf("ok: detached kernel driver\n");
	}

	err = libusb_claim_interface(device, 0);
	if(err) {
		printf("err: failed to claim interface: %s\n", libusb_error_name(err));
		return -1;
	}

	printf("ok: interface for device claimed\n");
	printf("ok: starting polling for interrupt data\n");

	unsigned char data[2];
	int transferred;
	while(!stop) {
		err = libusb_interrupt_transfer(device, 1 | LIBUSB_ENDPOINT_IN, data, 2, &transferred, 1);
		if(err == LIBUSB_ERROR_TIMEOUT) {
			continue;
		}else if(err) {
			if(err == LIBUSB_ERROR_NO_DEVICE) {
				printf("info: device disconnected\n");
			}else {
				printf("err: interrupt transfer failed: %s\n", libusb_error_name(err));
			}

			libusb_release_interface(device, 0);
			libusb_close(device);
			libusb_exit(usb_ctx);
			device = NULL;
			goto start;
		} else {
			printf("address: 0x%02X\n", data[0]);
			printf("command: 0x%02X\n", data[1]);

			if(commands[data[1]] != NULL) {
				system(commands[data[1]]);
			}
		}
	}

	printf("done\n");
	free_commands(commands);
	libusb_close(device);
	libusb_exit(usb_ctx);
	return 0;
}
