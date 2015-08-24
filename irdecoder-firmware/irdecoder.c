#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "irdecoder.h"
#include "nec.h"
#include "usart.h"

static volatile uint8_t new_cmd;
static volatile uint8_t cmd;
static volatile uint8_t addr;


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
		new_cmd = 1;
		cmd = nec_get_cmd();
		nec_reset();
	}

	TCCR1B = _BV(CS11)   |   /* CLK/8 prescaler, meaning timer increment every CLK/8 Hz */
			 (0 << CS12) |
			 (0 << CS10) |
			 _BV(ICNC1)  |   /* enable input capture noise canceler */
			 (0 << ICES1);  /* set capture edge selector to falling edge */

}

int main(void)
{

	/* configure the 16 bit Timer1/Counter1 which we will use as an
	 * input capture to measure the pulse widths of our IR signal */
	TCCR1B = _BV(CS11)   |   /* CLK/8 prescaler, meaning timer increment every CLK/8 Hz */
			 (0 << CS12) |
			 (0 << CS10) |
			 _BV(ICNC1)  |   /* enable input capture noise canceler */
			 (0 << ICES1);  /* set capture edge selector to falling edge */


	TIMSK1 |= _BV(TOIE1) | /* enable timer overflow interrupt */
			 _BV(ICIE1);  /* enable input capture interrupt */

	/* initialize decoder */
	nec_init();

	usart_init(USART_BAUD);

	/* enable interrupts and loop forever. all processing and communication
	 * happens in interrupt routines */
	sei();
    while(1) {
    	if(new_cmd == 1) {
    		usart_tx_str("CMD = ");
    		usart_tx_int(cmd);
    		usart_tx_str("\r\n");

    		usart_tx_str("ADDR = ");
    		usart_tx_int(addr);
    		usart_tx_str("\r\n");

    		new_cmd = 0;
    	}
    }
}
