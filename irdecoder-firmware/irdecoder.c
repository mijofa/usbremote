#define F_CPU   8000000L

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

/************************************************
 *          IR and USART configuration
 ************************************************/
#define IR_START_BIT    4000
#define IR_BIT_1        1500
#define IR_BIT_0        450
#define IR_MAX_BITS     33
#define IR_MAX_TRANS    (IR_MAX_BITS * 2)
#define IR_TIMEOUT      5000

#define USART_BAUD      (((F_CPU / (9600 * 16UL))) - 1)

static unsigned int transitions[IR_MAX_BITS];
static unsigned int recorded_transitions;
static unsigned int state;


/* 16 bit timer input capture interrupt -- jmps  when the ICP1
 * pin changes based on the configured edge
 */
ISR(TIMER1_CAPT_vect)
{
    static unsigned int previous_ts;
    unsigned int ts;
    unsigned int phase_len;
    
    /* record current timestamp */
    ts = ICR1;
    
    /* toggle edge selector, 0 is rising, 1 is falling */
    TCCR1B ^= _BV(ICES1);
    
    phase_len = ts - previous_ts;
    previous_ts = ts;
    
    /* set timeout value for comparator */
    OCR1A = ts + IR_TIMEOUT;
    
    phases[recorded_transitions++] = phase_len;
        
}

/* 16 bit timer comparator interrupt -- jmps any time the timer hits the timeout value
 * given after each phase change from input capture
 */
ISR(TIMER1_COMPA_vect)
{
    if(recorded_transitions == IR_MAX_TRANS) {
        /* TODO: turn off IR interrupts */
        state = 1;
    } else {
        recorded_transitions = 0;
    }
}

uint32_t trans_to_uint32()
{ 
    
}

int main(void)
{
    usart_init(USART_BAUD);
    
    uint32_t ir_code;
    
    while(1) {
    
        if(state == 1) {
            ir_code = trans_to_uint32();
            char ir_code_str[11];
            ultoa(ir_code, ir_code_str, 10);
            usart_tx_str(ir_code_str, strlen(ir_code_str));
            usart_tx('\n');
            
            state = 0;
            recorded_transitions = 0;
            /* TODO: turn IR interrupts back on */
        }
    
    }
    
}





