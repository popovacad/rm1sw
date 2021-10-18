#include "checksum.h"

u8 CalcXor(u8 *buf, u8 len)
{
  u8 sum = 0;
  for (u8 i = 0; i < len; i++)
  {
    sum = sum ^ *(buf+i);
  }
  return sum;
}
