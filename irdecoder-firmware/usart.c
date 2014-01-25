#include "usart.h"
#include <avr/io.h>


void usart_init(unsigned int baud)
{
	UBRRH = (unsigned char)(baud >> 8);
	UBRRL = (unsigned char)baud;

	UCSRB = (1 << RXEN) | (1 << TXEN);
	UCSRC = (0 << USBS) | (1 << UCSZ1) | (1 << UCSZ0);
}

void usart_tx(unsigned char byte)
{
	while( !(UCSRA & (1 << UDRE)) );
	UDR = byte;
}

unsigned char usart_rx(void)
{
	while( !(UCSRA & (1 << RXC)) );
	return UDR;
}

