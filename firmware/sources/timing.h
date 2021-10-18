#ifndef __TIMING_H
#define __TIMING_H

#include "stm8s.h"

void InitTiming();
void Delay1(u16 ms);
void Delay2(u8 x100us);

#endif /* __TIMING_H */
