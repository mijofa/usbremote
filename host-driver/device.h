#ifndef _DEVICE_H_
#define _DEVICE_H_

struct usbremote_packet {
	uint8_t addr;
	uint8_t cmd;
};

libusb_device_handle *open_device(libusb_context *, uint16_t, uint16_t);
void close_device(libusb_device_handle *device);
int read_remote_data(libusb_device_handle *, struct usbremote_packet *, int);

#endif
