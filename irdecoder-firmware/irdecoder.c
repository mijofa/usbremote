#ifndef F_CPU
#define F_CPU 20000000L
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "irdecoder.h"
#include "nec.h"

/* 16 bit timer input capture interrupt -- jmps  when the ICP1
 * pin changes based on the configured edge
 */
ISR(TIMER1_CAPT_vect)
{
    uint16_t ts;
    
    /* record current timestamp */
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
 * 26 milliseconds.
 */
ISR(TIMER1_OVF_vect)
{
	nec_decode();

	uint8_t decoder_state;
	decoder_state = nec_get_state();
	if(decoder_state == STATE_SUCCESSFUL_READ) {
		// TODO: send the address and command somewhere
		nec_reset();
	}
}

int main(void)
{

	/* configure the 16 bit Timer1/Counter1 which we will use as an
	 * input capture to measure the pulse widths of our IR signal */
	TCCR1B = _BV(CS11)  | /* CLK/8 prescaler, meaning timer incrememt every CLK/8 Hz */
			 _BV(ICNC1) | /* enable input capture noise canceler */
			 (0 << ICES1);  /* set capture edge selector to falling edge */


	TIMSK |= _BV(TOIE1) | /* enable timer overflow interrupt */
			 _BV(ICIE1);  /* enable input capture interrupt */

	/* initialize decoder */
	nec_init();

	/* enable interrupts and loop forever. all processing and communication
	 * happens in interrupt routines */
	sei();
    while(1);
}
