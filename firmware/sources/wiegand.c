#include "wiegand.h"

#include "timing.h"

#include "device.h"

#define DATA0_LO() GPIO_WriteLow(DATA0_PORT, DATA0_PIN)
#define DATA0_HI() GPIO_WriteHigh(DATA0_PORT, DATA0_PIN)
#define DATA1_LO() GPIO_WriteHigh(DATA1_PORT, DATA1_PIN)
#define DATA1_HI() GPIO_WriteLow(DATA1_PORT, DATA1_PIN)

void InitWiegand()
{
  GPIO_Init(DATA0_PORT, DATA0_PIN, GPIO_MODE_OUT_PP_HIGH_SLOW);
  DATA0_HI();
  GPIO_Init(DATA1_PORT, DATA1_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  DATA1_HI();
}

void OutW(u8 bit, u8 pulse, u8 pause)
{
  if (bit)
  {
    DATA1_LO();
    Delay2(pulse);
    DATA1_HI();
  }
  else
  {
    DATA0_LO();
    Delay2(pulse);
    DATA0_HI();
  }

  Delay2(pause);
}

u8 GetBit(u8 *buf, u8 bit)
{
  return ((buf[bit/8] << (bit%8)) & 0x80) >> 7;
}

void Wiegand(u8 *buf, u8 len, u8 sum, u8 pulse, u8 pause)
{
  u8 leftsum = 0, rightsum = 1;

  // Calculate checksum if applicable
  if (sum)
  {
    for (u8 i = 0; i < len*8/2; i++)
      leftsum ^= GetBit(buf, i);
    for (u8 i = len*8/2; i < len*8; i++)
      rightsum ^= GetBit(buf, i);
  }

  // Strange kostyl!
  // Without this first 100us pause will be significantly smaller
  Delay2(1);

  // Send wiegand packet
  if (sum)
    OutW(leftsum, pulse, pause);
  for (u8 i = 0; i < len*8; i++)
    OutW(GetBit(buf, i), pulse, pause);
  if (sum)
    OutW(rightsum, pulse, pause);
}
