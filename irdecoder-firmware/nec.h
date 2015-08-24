#ifndef _NEC_H_
#define _NEC_H_

/* number of clock cycles to 1 millisecond. this needs to be
 * recalculated if prescaler is changed or clock speed changes.
 * currently set for 20MHz clock with CLK/8 prescaler*/
#define ONE_MS		2500 /* 26ms overflow */

/* maximum amount of edges and edge counts for proper packets */
#define NEC_MAX_EDGES 					68
#define NEC_PACKET_EDGE_COUNT			68
#define NEC_REPEAT_PACKET_EDGE_COUNT	4

/* acceptable error margin to account for variations in pulse widths */
#define NEC_PULSE_ERROR_MARGIN			.15

/* 9ms start pulse */
#define NEC_START_PULSE_MIN				((ONE_MS * 9) - (ONE_MS * NEC_PULSE_ERROR_MARGIN))
#define NEC_START_PULSE_MAX				((ONE_MS * 9) + (ONE_MS * NEC_PULSE_ERROR_MARGIN))
#define NEC_POST_START_PAUSE_MIN		((ONE_MS * 4.5) - (ONE_MS * NEC_PULSE_ERROR_MARGIN))
#define NEC_POST_START_PAUSE_MAX		((ONE_MS * 4.5) + (ONE_MS * NEC_PULSE_ERROR_MARGIN))
#define NEC_BIT_PULSE_MIN				((ONE_MS * 0.56) - (ONE_MS * NEC_PULSE_ERROR_MARGIN))
#define NEC_BIT_PULSE_MAX				((ONE_MS * 0.56) + (ONE_MS * NEC_PULSE_ERROR_MARGIN))
#define NEC_1_BIT_TRANSMIT_TIME_MIN		((ONE_MS * 1.125) - (ONE_MS * NEC_PULSE_ERROR_MARGIN))
#define NEC_1_BIT_TRANSMIT_TIME_MAX		((ONE_MS * 2.25) + (ONE_MS * NEC_PULSE_ERROR_MARGIN))

/* command codes */
#define NEC_COMMAND_REPEAT	0xFF

/* NEC decoding states */
#define STATE_NONE						0x00
#define STATE_RECEIVED_START_PULSE		0x01
#define STATE_RECEIVED_ADDRESS			0x02
#define STATE_RECEIVED_ADDR_INVERSE		0x03
#define STATE_RECEIVED_COMMAND			0x04
#define STATE_RECEIVED_COMMAND_INVERSE	0x05
#define STATE_SUCCESSFUL_READ			0x06


/* function prototypes */
void nec_init(void);
void nec_reset(void);
void nec_add_edge(uint16_t);
uint8_t nec_get_addr(void);
uint8_t nec_get_cmd(void);
uint8_t nec_get_state(void);
void nec_decode(void);
uint8_t __nec_decode_byte(uint8_t);

#endif
