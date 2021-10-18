#ifndef __SW_UART_H
#define __SW_UART_H

#include "stm8s.h"

#define RX_ERROR 4
#define RX_BUFF_OVFL 2
#define RX_BUFF_FULL 1

extern u8 swutxbuf[64];
extern u8 swurxbuf[64];

void SwUInit();
void SwUTick();
u8 SwUTx(u8 len, u16 to);
u8 SwURx(u16 to);

#endif /* __SW_UART_H */
