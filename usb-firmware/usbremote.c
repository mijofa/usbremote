#define F_CPU   20000000L

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdint.h>
#include "usbdrv/usbdrv.h"
#include "usbrequests.h"


USB_PUBLIC unsigned char usbFunctionSetup(unsigned char data[8])
{
    usbRequest_t *request = (void *)data;
    static uint32_t response;
    
    if(request->bRequest == USB_RQ_HELO) {
        response = USB_RSP_HELO;
        usbMsgPtr = response;
        return sizeof(uint32_t);
    }
    
    return USB_NO_MSG;
}

int main(void)
{
    wdt_enable(WDTO_1S);
    usbInit();
    usbDeviceDisconnect();
    
    int t;
    for(t = 0; t < 250; t++) {
        wdt_reset();
        _delay_ms(2);
    }
    
    usbDeviceConnect();
    sei();
    
    while(1) {
        wdt_reset();
        usbPoll();
    }
        
}


