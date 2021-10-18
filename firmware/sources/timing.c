#include "timing.h"

#include "indication.h"
#include "swuart.h"

u8 d10ms = 0;

u16 d1p = 0;

u8 d2p = 0;

INTERRUPT_HANDLER(TIM4_UPD_OVF_IRQHandler, ITC_IRQ_TIM4_OVF)
{
  if (TIM4_GetFlagStatus(TIM4_FLAG_UPDATE))
  {
    if (d10ms++ == 10)
    {
      // Process Delay1 (milliseconds)
      if (d1p > 0) d1p--;

      // Process SW UART
      SwUTick();

      // Process indication
      IndicationTick();
      
      d10ms = 0;
    }

    // Process Delay2 (x100 microseconds)
    if (d2p > 0) d2p--;

    // Clear flag (rearm interrupt)
    TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  }
}

void InitTiming()
{
  // Timer 4 as system tick, f = 10 kHz (period of 25 = 100us)
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4, ENABLE);
  TIM4_TimeBaseInit(TIM4_PRESCALER_64, 25);
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
  TIM4_Cmd(ENABLE);
}

void Delay1(u16 ms)
{
  d1p = ms;
  while (d1p > 0)
  {
#if !DEBUG
    __wait_for_interrupt();
#endif
  }
}

void Delay2(u8 x100us)
{
  d2p = x100us;
  while (d2p > 0)
  {
#if !DEBUG
    __wait_for_interrupt();
#endif
  }
}
