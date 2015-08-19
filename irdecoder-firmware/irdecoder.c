#ifndef F_CPU
#define F_CPU 20000000L
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "irdecoder.h"

static uint8_t edges[MAX_EDGES];
static uint8_t recorded_edges;
static uint8_t state;

static uint8_t address;
static uint8_t command;


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

    /* if this is the first edge we are recording then the time stamp
     * doesn't matter. in theory it should be the falling edge of the
     * start pulse */
    if(recorded_edges == 0) {
    	edges[recorded_edges++] = 0;
    } else {
    	edges[recorded_edges] = ts;
    }

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
	/* if the number of edges recorded doesn't match a complete command packet
	 * or repeat packet then we reset state and edge count */
	if(recorded_edges != FULL_PACKET_EDGES || recorded_edges != REPEAT_PACKET_EDGES) {
		recorded_edges = 0;
		state = STATE_NONE;
	}

	/* iterate through each edge to process the signal. we use a state machine
	 * to keep track of the process. note that in some cases we break out of the
	 * loop by setting the recorded_edges value to 0, usually after an error case.
	 * we do recorded_edges - 1 because for each state we are depending on the next
	 * edge for pulse calculations */
	uint16_t next_edge;
	uint16_t edge_index;
	for(edge_index = 0; edge_index < recorded_edges - 1; edge_index++) {
		next_edge = edges[edge_index + 1];

		switch(state) {

			case STATE_NONE:
				/* if the current pulse matches a start pulse then jump to the
				 * starting edge of the next phase and set our state to reflect that */
				if(next_edge >= NEC_START_PULSE_MIN && next_edge <= NEC_START_PULSE_MAX) {
					edge_index += 2;
					state = STATE_RECEIVED_START_PULSE;
				} else { /* not a start pulse, so reset */
					recorded_edges = 0;
				}

				break;

			case STATE_RECEIVED_START_PULSE:
				/* the next pulse should be the first bit of the address  */
				if(next_edge >= NEC_BIT_PULSE_MIN && next_edge <= NEC_BIT_PULSE_MAX) {
					/* if we have one bit pulse and no more, then this is a repeat code */
					if(edge_index + 2 < recorded_edges) {
						/* we don't set the address because address currently holds the
						 * last address, and a repeat code is intended for the same one */
						command = NEC_COMMAND_REPEAT;
						state = STATE_SUCCESSFUL_READ;
					} else {
						/* if we don't have 16 more edges then something is wrong, because
						 * the address alone is 8 bits, 16 edges */
						if(recorded_edges < edge_index + 16) {
							state = STATE_NONE;
							recorded_edges = 0;
							break;
						}

						uint8_t addr;
						addr = 0;
						/* the next eight pulses are the address, LSB first */
						int x;
						for(x = edge_index; x < edge_index + 8; x++) {
							next_edge = edges[x + 1];
							if(next_edge < NEC_BIT_PULSE_MIN || next_edge > NEC_BIT_PULSE_MAX) {
								state = STATE_NONE;
								recorded_edges = 0;
								break;
							}

							// TODO: shift in the current bit

						}
					}
				} else { /* wasn't a bit pulse that we expected */
					state = STATE_NONE;
					recorded_edges = 0;
					break;
				}

				break;

			case STATE_RECEIVED_ADDRESS:
				// TODO: shift in address
				break;

			case STATE_RECEIVED_ADDR_INVERSE:
				// TODO: shift in inverse of the address
				// TODO: compare inverse match to already received address
				break;

			case STATE_RECEIVED_COMMAND:
				break;

			case STATE_RECEIVED_COMMAND_INVERSE:
				break;

			case STATE_SUCCESSFUL_READ:
				break;
		}
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

	state = STATE_NONE;

	/* enable interrupts and loop forever. all processing and communication
	 * happens in interrupt routines */
	sei();
    while(1);
}
