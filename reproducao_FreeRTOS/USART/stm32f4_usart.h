#ifndef __STM32F4_UART_H
#define __STM32F4_UART_H

void USART_INIT(u32 baudrate);
void USART_Print(u8 *buf, u16 length);

#endif
