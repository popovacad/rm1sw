#ifndef __CONFIG_H
#define __CONFIG_H

#include "stm8s.h"
#include "string.h"

#include "checksum.h"

#define CFG_VER 1

// Configuration structure stuff
#define CFG_KEYT_A 0
#define CFG_KEYT_B 1

#define W_TYPE_AUTO 0
#define W_TYPE_24BIT 1

// Configuration structure
struct s_config
{
  u8 cfg_magic[4];
  u8 cfg_ver;

  u8 mf_m_keyt;    // 0 = A, 1 = B
  u8 mf_m_key[6];  // Access key
  u8 mf_m_sblock;  // Start block of magic, 0 to 63
  u8 mf_m_blocks;  // Number of blocks to read, 1 to 4; need to check sector boudary!
  u8 mf_m_pos;     // Position of magic in read content, 0 to (mf_m_blocks*16)-mf_m_length
  u8 mf_m_length;  // Length of magic, 1 to 4, 0 = disable magic verification
  u8 mf_m_data[4]; // Magic to compare with

  u8 mf_d_keyt;
  u8 mf_d_key[6];
  u8 mf_d_sblock;
  u8 mf_d_blocks;
  u8 mf_d_pos;
  u8 mf_d_length;  // Length of data, 3 (Wiegand-24/26) to 8 (Wiegand-64/66), 0 = disable reading secure data, use raw UID

  u8 w_pulse;      // Wiegand pulse length in 100uS resolution (Pulse in uS = w_pulse*100)
  u8 w_pause;      // Wiegnad pause between pulses in 100uS resolution
  u8 w_type;       // Wiegand type, 0 for auto, 1 = fixed 24 bits (26 with parity); setting most significant bit (mask 0x80) means to add parity, clearing means no parity added

  u8 checksum;
}; // 34 ?

// Instance of structure
extern struct s_config config;

// Exported functions
void InitConfig();
u8 CheckConfig();
void ReadConfig();
void WriteConfig();

#endif /* __CONFIG_H */
