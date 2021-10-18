#ifndef __MUSIC_H
#define __MUSIC_H

#include "stm8s.h"

#define MEL_DOREMIDO 0
#define MEL_START 1
#define MEL_NORMAL 2
#define MEL_LETA1 20
#define MEL_LETA2 21

void PlayMelody(u8 mel_id);

#endif /* __MUSIC_H */
