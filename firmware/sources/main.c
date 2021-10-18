#include "stm8s.h"
#include "string.h"

#include "checksum.h"
#include "config.h"
#include "indication.h"
#include "RDM880.h"
#include "swuart.h"
#include "timing.h"
#include "wiegand.h"
#include "music.h"

#include "device.h"

typedef enum
{
  MODE_NORMAL,
  MODE_CARDPROG
} CfgMode;

void DeInitGPIO()
{
  GPIO_DeInit(GPIOA);
  GPIO_DeInit(GPIOB);
  GPIO_DeInit(GPIOC);
  GPIO_DeInit(GPIOD);
  GPIO_DeInit(GPIOE);
  GPIO_DeInit(GPIOF);
  GPIO_DeInit(GPIOG);
}

CfgMode GetMode()
{
  DeInitGPIO();

  GPIO_Init(CFG_PORT, CFG_PIN, GPIO_MODE_IN_PU_NO_IT);

  if ((CFG_PORT->IDR & CFG_PIN) == 0)
    return MODE_CARDPROG;
  else
    return MODE_NORMAL;
}

void InitClock()
{
  // Master clock
  //CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
  CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);
  CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSE, DISABLE, CLK_CURRENTCLOCKSTATE_ENABLE);
}

void InitPlatform()
{
  // Disable interrupts
  __disable_interrupt();

  // Deinit all GPIOs !!!
  DeInitGPIO();

  // Init indication
  InitIndication();
  // Init timing subsys
  InitTiming();
  // Init SW UART
  SwUInit();
  // Init config subsys
  InitConfig();
  // Init wiegand output
  InitWiegand();

  // Configure interrupt priorities
  ITC_SetSoftwarePriority(ITC_IRQ_TIM4_OVF, ITC_PRIORITYLEVEL_2); // !!! IMPORTANT! It's needed to not interrupt SW UART by timing functions !!!
  //ITC_SetSoftwarePriority(ITC_IRQ_TIM2_OVF, ITC_PRIORITYLEVEL_2); // !!! IMPORTANT! It's needed to not interrupt SW UART by buzzer sound generation !!!

  // Enable interrupts
  __enable_interrupt();
}

void DoError(u8 blinks)
{
  for (;;)
  {
    RedLedCyc(100, 200, blinks);
    Delay1(2500);
  }
}

void DoCardProg()
{
  u8 o_uid[4] = {0x00, 0x00, 0x00, 0x00};
  u8 c_uid[4] = {0x00, 0x00, 0x00, 0x00};

  u8 key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  u8 data[52];
  u8 rres;

  for (;;)
  {
    BlueLedOn();
    Delay1(100);
    BlueLedOff();
    Delay1(500);
    
    if (RDM880_MifareClassic_DetectCard(&c_uid[0]) == 0 && memcmp(&c_uid[0], &o_uid[0], 4) != 0)
    {
      rres = RDM880_MifareClassic_ReadCard(4, 3, CFG_KEYT_A, &key[0], &data[0]);

      if (rres == 0)
      {
        if (memcmp(&c_uid[0], &data[0], 4) == 0)
        {
          memcpy((u8 *)(&config), &data[4], sizeof(config));
          if (CheckConfig() == 0)
          {
            WriteConfig();

            GreenLedOn();
            Delay1(1000);
            GreenLedOff();
            Delay1(1000);
          }
          else
          {
            RedLedCyc(100, 200, 2);
            Delay1(1000);
          }
        }
        else
        {
          RedLedCyc(100, 200, 3);
          Delay1(1000);
        }
      }
      else
      {
        RedLedCyc(100, 200, 1);
        Delay1(1000);
      }
    }
    memcpy(&o_uid, &c_uid, 4);
  }
}

void DoNormal()
{
  u8 o_uid[4] = {0x00, 0x00, 0x00, 0x00};
  u8 c_uid[4] = {0x00, 0x00, 0x00, 0x00};

  u8 data[68];
  u8 rres;

  u8 errcnt = 0;

  // Read configuration structure from flash
  ReadConfig();
  if (CheckConfig() != 0)
  {
    DoError(3); // Never return from here!
  }
  for (;;)
  {
    if (RDM880_MifareClassic_DetectCard(&c_uid[0]) == 0 && memcmp(&c_uid[0], &o_uid[0], 4) != 0)
    {
      // Read verification (if applicable)
      if (config.mf_m_length != 0)
      {
        rres = RDM880_MifareClassic_ReadCard(config.mf_m_sblock, config.mf_m_blocks, config.mf_m_keyt, &config.mf_m_key[0], &data[0]);
        if (rres != 0)
          goto except; // Some error, go to next iteration
        // Compare if against config
        if (memcmp(&data[4+config.mf_m_pos], &config.mf_m_data[0], config.mf_m_length) != 0)
          goto except; // Not identical, go to next iteration
      }
      // Read ID (if applicable)
      if (config.mf_d_length != 0)
      {
        if (config.mf_m_sblock != config.mf_d_sblock || config.mf_m_blocks != config.mf_m_blocks)
        {
          rres = RDM880_MifareClassic_ReadCard(config.mf_d_sblock, config.mf_d_blocks, config.mf_d_keyt, &config.mf_d_key[0], &data[0]);
          if (rres != 0)
            goto except; // Some error, go to next iteration
        }
        // Indicate OK
        BlueLedOn();
        Delay1(250);
        BlueLedOff();
        Delay1(100);
        // Output data as Wiegnad
        if ((config.w_type & 0x7F) == 0)
          Wiegand(&data[4+config.mf_d_pos], config.mf_d_length, config.w_type >> 7, config.w_pulse, config.w_pause);
        else
          Wiegand(&data[4+config.mf_d_pos+(config.mf_d_length-3)], 3, config.w_type >> 7, config.w_pulse, config.w_pause);
      }
      else // Output UID as Wiegand
      {
        // Indicate OK
        BlueLedOn();
        Delay1(250);
        BlueLedOff();
        Delay1(100);
        //
        if ((config.w_type & 0x7F) == 0)
          Wiegand(&data[0], 4, config.w_type >> 7, config.w_pulse, config.w_pause);
        else
          Wiegand(&data[1], 3, config.w_type >> 7, config.w_pulse, config.w_pause);
      }
    }
    errcnt = 0; // Reset error counter
    goto finally; // If no error, goto finally
except:
    if (errcnt == 3) {
      RedLedOn();
      PlayMelody(MEL_DOREMIDO);
      RedLedOff();
      errcnt = 0;
    }
    else
    {
      memset(&c_uid, 0, 4); // Reset UID because we need to retry from start in case of UID was read sucessfully
      Delay1(100);
      errcnt++;
    }
finally: // We have no try-except-finally in C, oy vey ;(
    memcpy(&o_uid, &c_uid, 4);
  }
}

void main(void)
{
  // Initialize master clock
  InitClock();

  // Get reader working mode
  CfgMode mode = GetMode();

  // Initialize platform
  InitPlatform();
  
  GreenLedOn();
  BlueLedOn();
  RedLedOn();

  // A bit of fun!
  // You must have two identical readers installed, say, on tripod device from both sides.
  // Uncomment next picece of code and for reader #1 use MEL_LETA1 when building firmware,
  // and for reader #2 use MEL_LETA2.
  // When you will turn two readers on synchronously, they will play "Mnogaya Leta"
  // by S. Prokofiev in two voices :)
  
  //RedLedCyc(50, 50, 30);
  //GreenLedCyc(75, 75, 20);
  //BlueLedCyc(100, 100, 15);
  //PlayMelody(MEL_LETA1);

  Delay1(250);

  GreenLedOff();
  BlueLedOff();
  RedLedOff();

  Delay1(250);

  switch (mode)
  {
    case MODE_NORMAL:
      PlayMelody(MEL_NORMAL);
      DoNormal(); // Never return!
      break;
    case MODE_CARDPROG:
      DoCardProg(); // Never return!
      break;
    default: // Should never be here!!!
      DoError(5);
  }
}
