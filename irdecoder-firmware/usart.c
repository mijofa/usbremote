#include "usart.h"
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>


void usart_init(unsigned int ubrr)
{
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UCSR0C = (0 << USBS0) | (1 << UCSZ01) | (1 << UCSZ00);
}

void usart_tx(unsigned char byte)
{
	while( !(UCSR0A & (1 << UDRE0)) );
	UDR0 = byte;
}

void usart_tx_blob(unsigned char *b, unsigned int len)
{
    int x;
    for(x = 0; x < len; x++) {
        usart_tx(b[x]);
    }
}

void usart_tx_str(char *s)
{
	while(*s) {
		usart_tx(*s++);
	}
}

void usart_tx_int(int i)
{
	char int_str[30];
	itoa(i, int_str, 10);
	usart_tx_str(int_str);
}

unsigned char usart_rx(void)
{
	while( !(UCSR0A & (1 << RXC0)) );
	return UDRE0;
}

int usart_tx_peek(void)
{
    return (UCSR0A & (1 << RXC0));
}
