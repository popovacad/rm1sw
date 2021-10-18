#include "music.h"

#include "timing.h"
#include "indication.h"

#include "notes_lengths.inc"
#include "melodies.inc"

void PlayMelody(u8 mel_id)
{
  const u16 * mel;
  u8 len = 0;

  switch (mel_id)
  {
  case MEL_DOREMIDO:
    mel = mel_doremido;
    len = sizeof(mel_doremido)/sizeof(u16);
    break;
  case MEL_START:
    mel = mel_start;
    len = sizeof(mel_start)/sizeof(u16);
    break;
  case MEL_NORMAL:
    mel = mel_normal;
    len = sizeof(mel_normal)/sizeof(u16);
    break;
  case MEL_LETA1:
    mel = mel_leta1;
    len = sizeof(mel_leta1)/sizeof(u16);
    break;
  case MEL_LETA2:
    mel = mel_leta2;
    len = sizeof(mel_leta2)/sizeof(u16);
    break;
  }

  for (u8 i = 0; i < len; i = i+2)
  {
    BuzzerOn(mel[i]);
    Delay1(mel[i+1]);
  }

  BuzzerOff();
}
