#include "RDM880.h"

#include "string.h"

#include "swuart.h"
#include "checksum.h"
#include "config.h"

u8 RDM880_ExecuteCommand(u8 command, u8 *txdata, u8 txdatalen, u8 *rxdata, u8 *rxdatalen, u16 rxto)
{
  u8 rxlen;

  // Prepare
  swutxbuf[0] = 0xAA; // SOP
  swutxbuf[1] = 0x00; // Address
  swutxbuf[2] = txdatalen+1; // Data lenght + 1 for command
  swutxbuf[3] = command; // Command
  if (txdatalen > 0)
    memcpy(&swutxbuf[4], txdata, txdatalen); // Data
  swutxbuf[4+txdatalen] = CalcXor(&swutxbuf[1], txdatalen+3); // Checksum
  swutxbuf[5+txdatalen] = 0xBB; // EOP
  //
  memset(&swurxbuf[0], 0x00, 64);
  // Transmit
  txdatalen += 6;
  if (SwUTx(txdatalen, 100) != txdatalen)
    return 0xFB; // Transmit timeout
  // Receive and wait
  rxlen = SwURx(rxto);
  // Check and parse
  if (rxlen == 0)
    return 0xFF; // No data at all
  if (swurxbuf[0] != 0xAA || swurxbuf[rxlen-1] != 0xBB)
    return 0xFE; // Invalid answer format
  if (swurxbuf[rxlen-2] != CalcXor(&swurxbuf[1], rxlen-3))
    return 0xFD; // Invalid checksum
  // Now it seems to be ok
  if (*rxdatalen < swurxbuf[2]-1)
    return 0xFC; // Insufficient receive data buffer
  if (swurxbuf[2] > 1) memcpy(rxdata, &swurxbuf[4], swurxbuf[2]-1); // Copy data
  *rxdatalen = swurxbuf[2]-1; // Return correct data length
  return swurxbuf[3]; // Return reader status
}

u8 RDM880_MifareClassic_DetectCard(u8 *uid)
{
  u8 gsnr[2];
  u8 ruid[5];
  u8 rxdlen = 0;
  u8 ans;

  memset(uid, 0x00, 4);

  gsnr[0] = 0x52; // Request all
  gsnr[1] = 0x01; // Execute HALT at end

  rxdlen = 5;
  ans = RDM880_ExecuteCommand(0x25, &gsnr[0], 2, &ruid[0], &rxdlen, 100);

  if (ans > 0)
    return ans;
  if (rxdlen != 5)
    return 0xEF;
  if (ruid[0] != 0x00)
    return 0xEE;

  memcpy(uid, &ruid[1], 4);

  return 0;
}

u8 RDM880_MifareClassic_ReadCard(u8 sblock, u8 blocks, u8 keyt, u8 *key, u8 *data)
{
  u8 rreq[9];
  u8 rxdlen = 0;
  u8 ans;

  if (keyt == CFG_KEYT_A)
  {
    rreq[0] = 0x01;
  }
  else if (keyt == CFG_KEYT_B)
  {
    rreq[0] = 0x03;
  }
  else
    return 0xFF;

  rreq[1] = blocks;
  rreq[2] = sblock;
  memcpy(&rreq[3], key, 6);

  rxdlen = 52;
  ans = RDM880_ExecuteCommand(0x20, &rreq[0], 9, data, &rxdlen, 100);

  return ans;
}
