#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "usbdrv.h"

#include <util/delay.h>

USB_PUBLIC uchar usbFunctionSetup(uchar data[8]) {
        return 0; // do nothing for now
}

int main() {

    wdt_enable(WDTO_1S); // enable 1s watchdog timer

    usbInit();

    /* Maybe this isn't the first boot since connecting to the host, perhaps we crashed & the watchdog reset us.
     * If this is the case we must tell the host to disconnect and clear their memory of how to address us before reconnecting to get a useful address.
     * http://vusb.wikidot.com/examples#toc5
     */
    /* AIUI this is also fixed by having the D- pullup on a configured data pin, but I'm going to keep it here anyway for defense-in-depth. */
    usbDeviceDisconnect();
    uchar i = 0;
    while(--i){         // fake USB disconnect for > 250 ms
        wdt_reset();    // if watchdog is active, reset it
        _delay_ms(1);   // library call -- has limited range
    }
    usbDeviceConnect();

    sei(); // Enable interrupts after re-enumeration

    while(1) {
        wdt_reset(); // keep the watchdog happy
        usbPoll();
    }

    return 0;
}
