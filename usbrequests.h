#ifndef _USBREQUESTS_H_
#define _USBREQUESTS_H_

/* message sent upon connection on host side */
#define USB_RQ_HELO     			0x00
#define USB_RSP_HELO    			0x01

/* messages sent to check if a command has been received */
#define USB_RQ_CMD_AVAILABLE		0x01
#define USB_RSP_CMD_AVAILABLE		0x01
#define USB_RSP_CMD_NOT_AVAILABLE	0x00

/* messages sent when requesting command code or address */
#define USB_RQ_CMD_CODE				0x02
#define USB_RQ_CMD_ADDR				0x03

#endif
