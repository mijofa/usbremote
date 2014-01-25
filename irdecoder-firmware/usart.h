#ifndef _USART_H_
#define _USART_H_

void usart_init(unsigned int);

void usart_tx(unsigned char);

unsigned char usart_rx(void);

#endif
