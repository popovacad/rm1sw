#include "config.h"

const u8 CFG_MAGIC[4] = {'F', 'U', 'C', 'K'}; // Fast Unified Configuration Kernel, of corsa :)

struct s_config config;

void InitConfig()
{
  // EEPROM
  FLASH_SetProgrammingTime(FLASH_PROGRAMTIME_STANDARD);
}

u8 CheckConfig()
{
  if (memcmp(&config.cfg_magic[0], &CFG_MAGIC[0], 4) != 0)
    return 0xFF; // Invalid config magic
  if (config.cfg_ver != CFG_VER)
    return 0xFE; // Invalid config version
  if (config.checksum != CalcXor((u8 *)&config, sizeof(config)-1))
    return 0xFD; // Invalid checksum

  if (config.mf_m_length > 0) // If 0 then we do not need to verify magic
  {
    if (config.mf_m_length > 4)
      return 0xFC; // Invalid length
    if (config.mf_m_keyt > 1)
      return 0xFC; // Invalid key type
    if (config.mf_m_sblock > 63)
      return 0xFC; // Invalid start block
    if (config.mf_m_blocks < 1 || config.mf_m_blocks > 4)
      return 0xFC; // Invalid number of blocks to read
    // Sector boundary check
    if ((config.mf_m_sblock / 4) != ((config.mf_m_sblock+(config.mf_m_blocks-1)) / 4))
      return 0xFC; // Blocks to read crossing sector boundary
    if (config.mf_m_pos > (config.mf_m_blocks*16)-config.mf_m_length)
      return 0xFC; // Position is out of read band
  }

  if (config.mf_d_length > 0) // If 0 then we do not need to read secure data
  {
    if (config.mf_d_length < 3 || config.mf_d_length > 8)
      return 0xFC; // Invalid length
    if (config.mf_d_keyt > 1)
      return 0xFC; // Invalid key type
    if (config.mf_d_sblock > 63)
      return 0xFC; // Invalid start block
    if (config.mf_d_blocks < 1 || config.mf_d_blocks > 4)
      return 0xFC; // Invalid number of blocks to read
    // Sector boundary check
    if ((config.mf_d_sblock / 4) != ((config.mf_d_sblock+(config.mf_d_blocks-1)) / 4))
      return 0xFC; // Blocks to read crossing sector boundary
    if (config.mf_d_pos > (config.mf_d_blocks*16)-config.mf_d_length)
      return 0xFC; // Position is out of read band
  }

  if ((config.w_type & 0x0F) > 1)
    return 0xFC; // Invalid Wiegand type (only 0 = auto and 1 = fixed 24-bits accepted)

  // Does we need to check w_type binary flags??

  if (config.w_pulse < 1 || config.w_pulse > 5) // 100-500 uS
    return 0xFC; // Invalid Wiegand pulse length
  if (config.w_pause < 10 || config.w_pause > 50) // 1000-5000 uS
    return 0xFC; // Invalid Wiegand pause length

  return 0;
}

void ReadConfig()
{
  for (u8 i = 0; i < sizeof(config); i++)
  {
    *(&config.cfg_magic[0]+i) = FLASH_ReadByte(FLASH_DATA_START_PHYSICAL_ADDRESS+i);
  }
}

void WriteConfig()
{
  FLASH_Unlock(FLASH_MEMTYPE_DATA);
  while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET) {}

  for (u8 i = 0; i < sizeof(config); i++)
  {
    FLASH_ProgramByte(FLASH_DATA_START_PHYSICAL_ADDRESS+i, *(&config.cfg_magic[0]+i));
  }

  FLASH_Lock(FLASH_MEMTYPE_DATA);
}
