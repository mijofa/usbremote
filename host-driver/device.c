#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include "device.h"

libusb_device_handle *open_device(libusb_context *ctx, uint16_t vid, uint16_t pid)
{
	libusb_device_handle *device;
	device = NULL;

	device = libusb_open_device_with_vid_pid(ctx, vid, pid);
	if(device == NULL) {
		return NULL;
	}

	int err;

	err = libusb_set_configuration(device, 1);
	if(err) {
		libusb_close(device);
		return NULL;
	}

	if(libusb_kernel_driver_active(device, 1) == 1) {
		if((err = libusb_detach_kernel_driver(device, 0)) != LIBUSB_SUCCESS) {
			libusb_close(device);
			return NULL;
		}
	}

	err = libusb_claim_interface(device, 0);
	if(err) {
		libusb_close(device);
		return NULL;
	}

	return device;
}

void close_device(libusb_device_handle *device)
{
	libusb_release_interface(device, 0);
	libusb_close(device);
}

int read_remote_data(libusb_device_handle *d, struct usbremote_packet *p, int timeout) {
	int r;
	int read;
	uint8_t data[2];
	r = libusb_interrupt_transfer(d, 1 | LIBUSB_ENDPOINT_IN, data, 2, &read, timeout);
	if(r == LIBUSB_ERROR_TIMEOUT) {
		return 0;
	}

	if(r != LIBUSB_SUCCESS) {
		return -1;
	}

	p->addr = data[0];
	p->cmd = data[1];
	return 1;

}
