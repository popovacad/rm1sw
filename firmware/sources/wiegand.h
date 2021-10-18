#ifndef __WIEGAND_H
#define __WIEGAND_H

#include "stm8s.h"

void InitWiegand();
void Wiegand(u8 *buf, u8 len, u8 sum, u8 pulse, u8 pause);

#endif /* __WIEGAND_H */
