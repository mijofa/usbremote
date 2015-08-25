#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <libusb-1.0/libusb.h>

int main(int argc, char**argv)
{
	int err;
	uint16_t vid = 0x16C0;
	uint16_t pid = 0x05DC;

	libusb_context *usb_ctx;
	if(libusb_init(&usb_ctx) != LIBUSB_SUCCESS) {
		printf("err: failed to initialize libusb\n");
		return -1;
	}

	libusb_set_debug(usb_ctx, 3);

	libusb_device_handle *device;
	device = libusb_open_device_with_vid_pid(usb_ctx, vid, pid);
	if(device == NULL) {
		printf("err: failed to open device (vid=%04X pid=%04X)\n", vid, pid);
		return -1;
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
	while(1) {
		err = libusb_interrupt_transfer(device, 1 | LIBUSB_ENDPOINT_IN, data, 2, &transferred, 0);
		if(err) {
			printf("err: interrupt transfer failed: %s\n", libusb_error_name(err));
			break;
		}

		printf("address: 0x%02X\n", data[0]);
		printf("command: 0x%02X\n", data[1]);
	}

	libusb_close(device);
	libusb_exit(usb_ctx);
	return 0;
}
