#ifndef __INDICATION_H
#define __INDICATION_H

#include "stm8s.h"

#define INF_CYC 255

#define BUZ_DEFNOTE 9090 // ~880 Hz for A3 

void InitIndication();
void IndicationTick();

void GreenLedCyc(u16 onms, u16 offms, u8 cycles);
void GreenLedOn();
void GreenLedOff();

void BlueLedCyc(u16 onms, u16 offms, u8 cycles);
void BlueLedOn();
void BlueLedOff();

void RedLedCyc(u16 onms, u16 offms, u8 cycles);
void RedLedOn();
void RedLedOff();

void BuzzerCyc(u16 onms, u16 offms, u8 cycles, u16 timrel);
void BuzzerOn(u16 timrel);
void BuzzerOff();

#endif /* __INDICATION_H */
