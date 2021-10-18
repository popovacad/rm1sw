#ifndef __RDM880_H
#define __RDM880_H

#include "stm8s.h"

u8 RDM880_MifareClassic_DetectCard(u8 *uid);
u8 RDM880_MifareClassic_ReadCard(u8 sblock, u8 blocks, u8 keyt, u8 *key, u8 *data);

#endif /* __RDM880_H */
