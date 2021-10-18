#include "indication.h"

#include "device.h"

#define GLED_ON() GPIO_WriteHigh(GLED_PORT, GLED_PIN)
#define GLED_OFF() GPIO_WriteLow(GLED_PORT, GLED_PIN)
#define BLED_ON() GPIO_WriteHigh(BLED_PORT, BLED_PIN)
#define BLED_OFF() GPIO_WriteLow(BLED_PORT, BLED_PIN)
#define RLED_ON() GPIO_WriteHigh(RLED_PORT, RLED_PIN)
#define RLED_OFF() GPIO_WriteLow(RLED_PORT, RLED_PIN)

#define BUZ_TOG() GPIO_WriteReverse(BUZ_PORT, BUZ_PIN)
#define BUZ_OFF() GPIO_WriteLow(BUZ_PORT, BUZ_PIN)

#define TST_GLEDI ((GLEDI_PORT->IDR & GLEDI_PIN))
#define TST_RLEDI ((RLEDI_PORT->IDR & RLEDI_PIN))
#define TST_BUZI ((BUZI_PORT->IDR & BUZI_PIN))

// -- Green LED --
u16 glonms = 0;
u16 gloffms = 0;
u8 glcycles = 0;
u16 glcnt = 0;
u8 glst = 0;
//u8 glsem = 0;
u8 glman = 0;

// -- Blue LED --
u16 blonms = 0;
u16 bloffms = 0;
u8 blcycles = 0;
u16 blcnt = 0;
u8 blst = 0;
//u8 blsem = 0;
u8 blman = 0;

// -- Red LED --
u16 rlonms = 0;
u16 rloffms = 0;
u8 rlcycles = 0;
u16 rlcnt = 0;
u8 rlst = 0;
//u8 rlsem = 0;
u8 rlman = 0;

// -- Buzzer --
u16 bzonms = 0;
u16 bzoffms = 0;
u8 bzcycles = 0;
u16 bzcnt = 0;
u8 bzst = 0;
//u8 bzsem = 0;
u8 bzman = 0;
u16 bztr = 0;

INTERRUPT_HANDLER(TIM2_UPD_OVF_BRK_IRQHandler, ITC_IRQ_TIM2_OVF)
{
  if (TIM2_GetFlagStatus(TIM2_FLAG_UPDATE))
  {
    BUZ_TOG(); // Simply toggle buzzer pin

    TIM2_ClearFlag(TIM2_FLAG_UPDATE);
  }
}

void BuzzerPlay(u16 timrel)
{
  if (timrel == 0)
  {
    TIM2_Cmd(DISABLE);
    BUZ_OFF(); // Safety
  }
  else
  {
    TIM2_SetAutoreload(timrel);
    TIM2_Cmd(ENABLE);
  }
}

void InitIndication()
{
  // LEDs
  GPIO_Init(GLED_PORT, GLED_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  GPIO_Init(BLED_PORT, BLED_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  GPIO_Init(RLED_PORT, RLED_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  GLED_OFF();
  BLED_OFF();
  RLED_OFF();

  // Buzzer
  GPIO_Init(BUZ_PORT, BUZ_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
  BUZ_OFF();

  // Inputs
  // Really we need an additional external strong pull-ups here, I think about 1Kohm
  GPIO_Init(GLEDI_PORT, GLEDI_PIN, GPIO_MODE_IN_PU_NO_IT);
  GPIO_Init(RLEDI_PORT, RLEDI_PIN, GPIO_MODE_IN_PU_NO_IT);
  GPIO_Init(BUZI_PORT, BUZI_PIN, GPIO_MODE_IN_PU_NO_IT);
  
  // Timer 2 for buzzer
  TIM2_TimeBaseInit(TIM2_PRESCALER_1, BUZ_DEFNOTE); // ~880 Hz for A3
  TIM2_UpdateRequestConfig(TIM2_UPDATESOURCE_REGULAR);
  TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);
  TIM2_Cmd(DISABLE); // Initially disable
}

void IndicationTick()
{
  // Process Green LED
  if (TST_GLEDI == 0 || glman == 1)
  {
    glcycles = 0;
    GLED_ON();
  }
  else if (glcycles)
  {
    //glsem = 1;
    if (glcnt == 0)
    {
      if (glst == 0)
      {
        GLED_ON();
        glst = 1;
        glcnt = glonms;
      }
      else
      {
        GLED_OFF();
        glst = 0;
        glcnt = gloffms;
        if (glcycles > 0 && glcycles < 255) glcycles--;
      }
    }
    else
      glcnt--;
    //glsem = 0;
  }
  else
  {
    GLED_OFF();
  }

  // Process Blue LED
  if (blman == 1)
  {
    blcycles = 0;
    BLED_ON();
  }
  else if (blcycles)
  {
    //blsem = 1;
    if (blcnt == 0)
    {
      if (blst == 0)
      {
        BLED_ON();
        blst = 1;
        blcnt = blonms;
      }
      else
      {
        BLED_OFF();
        blst = 0;
        blcnt = bloffms;
        if (blcycles > 0 && blcycles < 255) blcycles--;
      }
    }
    else
      blcnt--;
    //blsem = 0;
  }
  else
  {
    BLED_OFF();
  }

  // Process Red LED
  if (TST_RLEDI == 0 || rlman == 1)
  {
    rlcycles = 0;
    RLED_ON();
  }
  else if (rlcycles)
  {
    //rlsem = 1;
    if (rlcnt == 0)
    {
      if (rlst == 0)
      {
        RLED_ON();
        rlst = 1;
        rlcnt = rlonms;
      }
      else
      {
        RLED_OFF();
        rlst = 0;
        rlcnt = rloffms;
        if (rlcycles > 0 && rlcycles < 255) rlcycles--;
      }
    }
    else
      rlcnt--;
    //rlsem = 0;
  }
  else
  {
    RLED_OFF();
  }

  // Process Buzzer
  if (TST_BUZI == 0 || bzman == 1)
  {
    bzcycles = 0;
    if (TST_BUZI == 0)
      bztr = BUZ_DEFNOTE;
    BuzzerPlay(bztr);
  }
  else if (bzcycles)
  {
    //bzsem = 1;
    if (bzcnt == 0)
    {
      if (bzst == 0)
      {
        BuzzerPlay(bztr);
        bzst = 1;
        bzcnt = bzonms;
      }
      else
      {
        BuzzerPlay(0);
        bzst = 0;
        bzcnt = bzoffms;
        if (bzcycles > 0 && bzcycles < 255) bzcycles--;
      }
    }
    else
      bzcnt--;
    //bzsem = 0;
  }
  else
  {
    BuzzerPlay(0);
  }
}

void GreenLedCyc(u16 onms, u16 offms, u8 cycles)
{
  GreenLedOff();
  glonms = onms;
  gloffms = offms;
  GLED_ON();
  glst = 1;
  glcnt = glonms;
  glcycles = cycles;
}

void GreenLedOn()
{
  glman = 1;
}

void GreenLedOff()
{
  glman = 0;
}

void BlueLedCyc(u16 onms, u16 offms, u8 cycles)
{
  BlueLedOff();
  blonms = onms;
  bloffms = offms;
  BLED_ON();
  blst = 1;
  blcnt = blonms;
  blcycles = cycles;
}

void BlueLedOn()
{
  blman = 1;
}

void BlueLedOff()
{
  blman = 0;
}

void RedLedCyc(u16 onms, u16 offms, u8 cycles)
{
  RedLedOff();
  rlonms = onms;
  rloffms = offms;
  RLED_ON();
  rlst = 1;
  rlcnt = rlonms;
  rlcycles = cycles;
}

void RedLedOn()
{
  rlman = 1;
}

void RedLedOff()
{
  rlman = 0;
}

// Buzzer

void BuzzerCyc(u16 onms, u16 offms, u8 cycles, u16 timrel)
{
  BuzzerOff();
  bzonms = onms;
  bzoffms = offms;
  bztr = timrel;
  BuzzerPlay(timrel);
  bzst = 1;
  bzcnt = rlonms;
  bzcycles = cycles;
}

void BuzzerOn(u16 timrel)
{
  bztr = timrel;
  bzman = 1;
}

void BuzzerOff()
{
  bzman = 0;
}
