#include <stdint.h>
#include "nec.h"

static uint8_t state;
static uint8_t edge_count;
static uint16_t edges[NEC_MAX_EDGES];

static uint8_t addr;
static uint8_t cmd;

/* nec_init intializes the internals for use */
void nec_init(void)
{
	addr = 0;
	cmd = 0;
	nec_reset();
}

/* nec_reset resets the decoder to the state present after calling nec_init. this is called
 * internally, but should also be called after calling nec_get_addr and nec_get_cmd after a
 * successful decode */
void nec_reset(void)
{
	state = STATE_NONE;
	edge_count = 0;

}


/* nec_add_edge adds an edge to the store for future processing. duration should be
 * a 16 bit integer representing the clock ticks since the last edge */
void nec_add_edge(uint16_t duration)
{
	if(edge_count < NEC_MAX_EDGES) {
		edges[edge_count++] = duration;
	}
}

/* nec_get_address returns the address from an NEC command. should be called
 * after a call to nec_decode if nec_get_state returns a successful read */
uint8_t nec_get_addr(void)
{
	return addr;
}

/* nec_get_cmd returns the command from an NEC command. should be called
 * after a call to nec_decode if nec_get_state returns a successful read */
uint8_t nec_get_cmd(void)
{
	return cmd;
}

/* nec_get_state returns the current state of the NEC decoder */
uint8_t nec_get_state(void)
{
	return state;
}

/* nec_decode takes all of the added edges and decodes it, extracting the
 * address and command. */
void nec_decode(void)
{
	if(edge_count != NEC_PACKET_EDGE_COUNT || edge_count != NEC_REPEAT_PACKET_EDGE_COUNT) {
		nec_reset();
		return;
	}

	uint8_t temp_addr;
	uint8_t temp_cmd;

	uint8_t edge_index;
	uint16_t next_edge;
	uint16_t curr_edge;
	for(edge_index = 0; edge_index < edge_count; edge_index++) {
		curr_edge = edges[edge_index];
		next_edge = edges[edge_index + 1];

		switch(state) {
			case STATE_NONE:
				if(next_edge >= NEC_START_PULSE_MIN && next_edge <= NEC_START_PULSE_MAX) {
					edge_index += 2;
					state = STATE_RECEIVED_START_PULSE;
				} else {
					nec_reset();
				}

				break;

			case STATE_RECEIVED_START_PULSE:
				/* immediately after the start pulse there is an extended low period. if it's not there
				 * then this signal is bad or doesn't belong to us */
				if(!(curr_edge >= NEC_POST_START_PAUSE_MIN && curr_edge <= NEC_POST_START_PAUSE_MAX)) {
					nec_reset();
					break;
				}

				/* we're expecting the first bit of the address here */
				if(!(next_edge >= NEC_BIT_PULSE_MIN && next_edge <= NEC_BIT_PULSE_MAX)) {
					nec_reset();
					break;
				}

				/* since it was a bit pulse, if there is no more pulses it's a repeat signal */
				if(edge_index + 2 > edge_count) {
					nec_reset();
					state = STATE_SUCCESSFUL_READ;
					break;
				}

				/* we should be at start edge of the first bit of the 8 bit address. so if
				 * there are less than 16 edges (8 bits) something is wrong */
				if(edge_count < edge_index + 16) {
					nec_reset();
					break;
				}

				/* decode the address byte */
				temp_addr = __nec_decode_byte(edge_index);

				state = STATE_RECEIVED_ADDRESS;
				edge_index += 16;
				break;

			case STATE_RECEIVED_ADDRESS:
				/* we're expecting the first bit of the inverse address byte */
				if(!(next_edge >= NEC_BIT_PULSE_MIN && next_edge <= NEC_BIT_PULSE_MAX)) {
					nec_reset();
					break;
				}

				/* we should be at start edge of the first bit of the 8 bit address inverse.
				 * so if there are less than 16 edges (8 bits) something is wrong */
				if(edge_count < edge_index + 16) {
					nec_reset();
					break;
				}

				/* read in the inverse address byte, invert it, and compare it to the address
				 * byte we already have. if it doesn't match we have an error */
				uint8_t inverse_addr;
				inverse_addr = __nec_decode_byte(edge_index);
				if(~inverse_addr != temp_addr) {
					nec_reset();
					break;
				}

				state = STATE_RECEIVED_ADDR_INVERSE;
				edge_index += 16;
				break;

			case STATE_RECEIVED_ADDR_INVERSE:
				/* we're expecting the first bit of the command byte */
				if(!(next_edge >= NEC_BIT_PULSE_MIN && next_edge <= NEC_BIT_PULSE_MAX)) {
					nec_reset();
					break;
				}

				/* we should be at start edge of the first bit of the 8 bit command.
				 * so if there are less than 16 edges (8 bits) something is wrong */
				if(edge_count < edge_index + 16) {
					nec_reset();
					break;
				}

				/* decode command byte */
				temp_cmd = __nec_decode_byte(edge_index);

				state = STATE_RECEIVED_COMMAND;
				edge_index += 16;
				break;

			case STATE_RECEIVED_COMMAND:
				/* we're expecting the first bit of the command inverse byte */
				if(!(next_edge >= NEC_BIT_PULSE_MIN && next_edge <= NEC_BIT_PULSE_MAX)) {
					nec_reset();
					break;
				}

				/* we should be at start edge of the first bit of the 8 bit inverse
				 * command. so if there are less than 16 edges (8 bits) something is wrong */
				if(edge_count < edge_index + 16) {
					nec_reset();
					break;
				}

				uint8_t inverse_cmd;
				inverse_cmd = __nec_decode_byte(edge_index);
				if(~inverse_cmd != temp_cmd) {
					nec_reset();
					break;
				}

				state = STATE_RECEIVED_COMMAND_INVERSE;
				edge_index += 16;
				break;

			case STATE_RECEIVED_COMMAND_INVERSE:
				/* we're expecting the stop bit */
				if(!(next_edge >= NEC_BIT_PULSE_MIN && next_edge <= NEC_BIT_PULSE_MAX)) {
					nec_reset();
					break;
				}

				addr = temp_addr;
				cmd = temp_cmd;
				state = STATE_SUCCESSFUL_READ;

		}
	}
}

uint8_t __nec_decode_byte(uint8_t offset)
{
	uint8_t addr;
	uint8_t x;
	uint16_t phase_end_ts;
	for(addr = 0, x = 0; x < 8; x++) {
		phase_end_ts = edges[offset * x + 2];
		if(phase_end_ts  > NEC_1_BIT_TRANSMIT_TIME_MIN) {
			addr |= (1 << (8 - x));
		}

	}

	return addr;
}
