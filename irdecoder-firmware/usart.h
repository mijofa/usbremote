#ifndef _USART_H_
#define _USART_H_

#include <stdint.h>

void usart_init(unsigned int);
void usart_tx(unsigned char);
void usart_tx_blob(unsigned char*, unsigned int);
void usart_tx_str(char *);
void usart_tx_int(int);
unsigned char usart_rx(void);
int usart_tx_peek(void);

#endif
