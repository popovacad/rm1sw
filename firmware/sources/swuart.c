// **************************************************** //
// *** PITA-free full duplex software UART for STM8 *** //
// *** Copyright (c) 2013-2014, Dark Simpson        *** //
// ***                                              *** //
// *** Used resources:                              *** //
// ***  - One timer with overflow interrupt and     *** //
// ***     output compare dummy (no pin change)     *** //
// ***     interrupt                                *** //
// ***  - One external pin interrupt                *** //
// **************************************************** //

#include "swuart.h"

#include "device.h"

#define SWUART_TXPORT RDM_TXPORT
#define SWUART_TXPIN RDM_TXPIN
#define SWUART_RXPORT RDM_RXPORT
#define SWUART_RXPIN RDM_RXPIN

#define SWUART_RXPORT_EXTI RDM_RXPORT_EXTI
#define SWUART_RXPORT_IRQ RDM_RXPORT_IRQ

// User must ensure calling service interrupt routine at exact intervals
// of 1 bit of the dedicated baud rate, so SWUART_BAUD must be defined correctly
#define SWUART_BAUD RDM_BAUD
#define SWUART_HALF_BAUD (SWUART_BAUD / 2)

#define test_rx (SWUART_RXPORT->IDR & SWUART_RXPIN)
#define set_tx (SWUART_TXPORT->ODR |= SWUART_TXPIN)
#define clr_tx (SWUART_TXPORT->ODR &= ~SWUART_TXPIN)

#define transmit_in_progress 0x80 // in progress mark
#define receive_in_progress 0x08 // in progress mark
#define receive_error 0x04
#define receive_buffer_overflow 0x02
#define receive_buffer_full 0x01

#define test_status(a) (swu_sts & a)
#define set_status(a) (swu_sts |= a)
#define clr_status(a) (swu_sts &= ~a)

#define enable_rxpin_int {\
  SWUART_RXPORT->CR2 |= (uint8_t)SWUART_RXPIN;\
}

#define disable_rxpin_int {\
  SWUART_RXPORT->CR2 &= (uint8_t)(~(SWUART_RXPIN));\
}

#define enable_rxoc_int {\
  TIM3->IER |= (uint8_t)TIM3_IT_CC1;\
}

#define disable_rxoc_int {\
  TIM3->IER &= (uint8_t)(~(TIM3_IT_CC1));\
}

const u8 MSK_TAB[8]= { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

u8 rx_bit, // counter of received bits
   tx_bit, // counter of transmited bits
   rx_buff, // receive byte buffer
   rx_data, // received byte register
   tx_data, // transmited byte register
   swu_sts; // SWUART status register

u8 swutxbuf[64];
u8 swutxlen, swutxpos = 0;

u8 swurxbuf[64];
u8 swurxpos = 0;

u8 swuart_tx_u8(u8 b);
u8 swuart_rx_u8(u8 *b);
void swuart_tx_timing(void);
void swuart_rx_start(void);
void swuart_rx_timing(void);

u16 dswrx = 0;

/* ------------- Interrupts ---------------- */

INTERRUPT_HANDLER(TIM3_UPD_OVF_BRK_IRQHandler, ITC_IRQ_TIM3_OVF)
{
  if (TIM3_GetFlagStatus(TIM3_FLAG_UPDATE))
  {
    swuart_tx_timing();

    TIM3_ClearFlag(TIM3_FLAG_UPDATE);
  }
}

INTERRUPT_HANDLER(EXTI_RXPORT_IRQHandler, SWUART_RXPORT_IRQ)
{
  // Now supposed that there is only one interrupt (from SW UART RX)
  swuart_rx_start();
}

INTERRUPT_HANDLER(TIM3_CAP_COM_IRQHandler, ITC_IRQ_TIM3_CAPCOM)
{
  if (TIM3_GetFlagStatus(TIM3_FLAG_CC1))
  {
    swuart_rx_timing();

    TIM3_ClearFlag(TIM3_FLAG_CC1);
  }
}

/* ------------- Exported ------------------ */
void SwUTick()
{
  // Process RX delay counter
  if (dswrx > 0) dswrx--;
}

void SwUInit()
{
  GPIO_Init(SWUART_TXPORT, SWUART_TXPIN, GPIO_MODE_OUT_PP_HIGH_SLOW);
  GPIO_Init(SWUART_RXPORT, SWUART_RXPIN, GPIO_MODE_IN_PU_IT);

  EXTI_SetExtIntSensitivity(SWUART_RXPORT_EXTI, EXTI_SENSITIVITY_FALL_ONLY);

  TIM3_TimeBaseInit(TIM3_PRESCALER_1, SWUART_BAUD);
  TIM3_ITConfig(TIM3_IT_UPDATE, ENABLE);
  TIM3_UpdateRequestConfig(TIM3_UPDATESOURCE_REGULAR);
  TIM3_OC1Init(TIM3_OCMODE_TIMING, TIM3_OUTPUTSTATE_DISABLE, 0, TIM3_OCPOLARITY_LOW); // RX OC
  TIM3_Cmd(ENABLE);
}

u8 SwUTx(u8 len, u16 to)
{
  u8 i = 0;
  for (; i < len; i++)
  {
    dswrx = to;
    while (swuart_tx_u8(swutxbuf[i]) == FALSE && dswrx > 0) {
#if !DEBUG
    //__wait_for_interrupt(); // ??? Is it really needed here ???
#endif
    }
    if (dswrx == 0)
      break;
  }
  return i;
}

u8 SwURx(u16 to)
{
  swurxpos = 0;
  dswrx = to;
  while (1)
  {
    while (swuart_rx_u8(&swurxbuf[swurxpos]) == FALSE && dswrx > 0) {
#if !DEBUG
    //__wait_for_interrupt(); // ??? Is it really needed here ??? 
#endif
    }
    if (dswrx == 0)
      return swurxpos;
    swurxpos++;
    dswrx = 10;
  }
}

/* ------------- Private ------------------- */

u8 swuart_tx_u8(u8 b)
{
  if (!test_status(transmit_in_progress))
  {
    tx_data = b;
    tx_bit = 0;
    set_status(transmit_in_progress);
    return(TRUE);
  }
  else
    return(FALSE);
}

u8 swuart_rx_u8(u8 *b)
{
  u8 res;
  if (test_status(receive_buffer_full))
  {
    *b = rx_data;
    res = swu_sts & 0x0F; // return only rx part of status
    clr_status(receive_error);
    clr_status(receive_buffer_full);
    clr_status(receive_buffer_overflow);
    return(res);
  }
  else
    return(FALSE);
}

void swuart_rx_start(void)
{
  if (!test_status(receive_in_progress))
  {
    // Disable GPIO interrupt
    disable_rxpin_int;
    // Init reception, set "receive in progress" status
    rx_bit = 0;
    set_status(receive_in_progress);
    // Configure output compare point
    uint16_t cnt = TIM3_GetCounter();
    TIM3_SetCompare1((SWUART_BAUD-cnt > SWUART_HALF_BAUD) ? cnt+SWUART_HALF_BAUD : cnt-SWUART_HALF_BAUD);
    // Clear false pending OC interrupt (if exist)
    TIM3_ClearFlag(TIM3_FLAG_CC1);
    // Start output compare interrupts
    enable_rxoc_int;
  }
}

void swuart_tx_timing(void)
{
  if (test_status(transmit_in_progress))
  {
    switch (tx_bit)
    {
      case 0:
        clr_tx;
        break;
      case 9:
        set_tx;
        break;
      case 10:
        clr_status(transmit_in_progress);
        break;
      default:
        if (tx_data & MSK_TAB[tx_bit-1])
          set_tx;
        else
          clr_tx;
    };
   tx_bit++;
  }
}

void swuart_rx_timing(void)
{
  if (test_status(receive_in_progress))
  {
    u8 rx_samp = test_rx;
    if (rx_bit == 0) // start bit!
    {
      if (rx_samp == 0) // correctly received, continue
      {
        rx_bit = 1;
        rx_buff = 0;
      }
      else // noise in start bit, find next one
      {
        disable_rxoc_int;
        clr_status(receive_in_progress);
        enable_rxpin_int;
      }
    }
    else
    {
      if (rx_bit <= 8) // bit <= 8, receive data bits
      {
        if (rx_samp)
          rx_buff |= MSK_TAB[rx_bit-1];
        rx_bit++;
      }
      else // bit > 8, receive stop bit
      {
        if (rx_samp) // valid stop bit
        {
          if (!test_status(receive_buffer_full))
          {
            rx_data = rx_buff;
            set_status(receive_buffer_full);
          }
          else
            set_status(receive_buffer_overflow);
        }
        else
        {
          rx_data = 0x00;
          set_status(receive_error);
        }
        disable_rxoc_int;
        clr_status(receive_in_progress);
        enable_rxpin_int;
      }
    }
  }
}
