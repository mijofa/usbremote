#define F_CPU   20000000L

#include <stdint.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "usbdrv/usbdrv.h"
#include "nec.h"


static volatile uint8_t cmd_available;
static volatile uint8_t cmd;
static volatile uint8_t addr;

USB_PUBLIC unsigned char usbFunctionSetup(unsigned char data[8])
{
    return USB_NO_MSG;
}

/* 16 bit timer input capture interrupt -- rjmps  when the ICP1
 * pin changes based on the configured edge
 */
ISR(TIMER1_CAPT_vect)
{
    uint16_t ts;

    /* record current timestamp. the input capture copies the value of
     * TCNT1 to ICR1 when a capture event occurs */
    ts = ICR1;

    /* toggle edge selector, 0 is rising, 1 is falling */
    TCCR1B ^= _BV(ICES1);

    nec_add_edge(ts);

    /* reset timer counter to prevent overflow and so next reading will
     * be an absolute time since last edge. no math required */
    TCNT1 = 0;
}

/* 16 bit timer overflow interrupt -- rjmp's any time timer1 overflows.
 * since we are set to CLK/8 and running at 20MHz, overflow happens after
 * 26 milliseconds. 26 milliseconds is kind of a sweet spot:
 *      * greater than any expected high or low phase
 *      * less than the time between a command and a repeat signal
 */
ISR(TIMER1_OVF_vect)
{

	nec_decode();

	uint8_t decoder_state;
	decoder_state = nec_get_state();
	if(decoder_state == STATE_SUCCESSFUL_READ) {
		cmd_available = 1;
		cmd = nec_get_cmd();
		addr = nec_get_addr();
		nec_reset();
	}

	/* reset the timer. we really just need to reset the edge selector but this wont hurt  */
	TCCR1B = _BV(CS11)   |  /* CLK/8 prescaler, meaning timer increment every CLK/8 Hz */
			 (0 << CS12) |
			 (0 << CS10) |
			 _BV(ICNC1)  | /* enable input capture noise canceler */
			 (0 << ICES1); /* set capture edge selector to falling edge */

}

int main(void)
{
	/* enable watchdog timer set to one second */
    wdt_enable(WDTO_1S);

    /* intialize USB driver */
    usbInit();

    /* we disconnect usb and pause for 250ms, and then connect to stabilize.
     * long explanation is: if a watchdog reset happens, the USB bus won't know it. so
     * it will continue to address us on our old address, even though we reset. so
     * doing this forces a re-enumeration to prevent this issue. */
    usbDeviceDisconnect();
    int t;
    for(t = 0; t < 250; t++) {
        wdt_reset();
        _delay_ms(2);
    }
    
    usbDeviceConnect();

	/* configure the 16 bit Timer1/Counter1 which we will use as an
	 * input capture to measure the pulse widths of our IR signal */
	TCCR1B = _BV(CS11)   |  /* CLK/8 prescaler, meaning timer increment every CLK/8 Hz */
			 (0 << CS12) |
			 (0 << CS10) |
			 _BV(ICNC1)  | /* enable input capture noise canceler */
			 (0 << ICES1); /* set capture edge selector to falling edge */


	TIMSK1 |= _BV(TOIE1) | /* enable timer overflow interrupt */
			 _BV(ICIE1);  /* enable input capture interrupt */

	/* initialize decoder */
	nec_init();

	/* udpate global status register to enable interrupts */
	sei();
    
    while(1) {
    	/* reset watchdog timer so we don't get reset */
        wdt_reset();

        /* poll USB -- this call must happen once every ~250ms */
        usbPoll();

        /* if the interrupt endpoint is ready to recieve data, and a new
         * IR command is ready, set the interrupt two a 2 byte chunk:
         *	- byte 1: address
         *	- byte 2: command
         */
        if(usbInterruptIsReady() && cmd_available) {
        	uint8_t chunk[2];
        	chunk[0] = addr;
        	chunk[1] = cmd;
        	usbSetInterrupt(chunk, 2);
        	cmd_available = 0;
        }
    }
        
}


