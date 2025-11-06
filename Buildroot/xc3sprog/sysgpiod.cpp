#include "sysgpiod.h"

#include <gpiod.h>

IOSysGPIOD::IOSysGPIOD (int tms, int tck, int tdi, int tdo)
 : TMSPin(tms), TCKPin(tck), TDIPin(tdi), TDOPin(tdo), chipname("gpiochip0")
{
    /*
    struct gpiod_chip *chip;
    struct gpiod_line *TMS;
    struct gpiod_line *TDI;
    struct gpiod_line *TCK;
    struct gpiod_line *TDO;
    */
    chip = gpiod_chip_open_by_name(chipname);
    TMS = gpiod_chip_get_line(chip,TMSPin);
    TDI = gpiod_chip_get_line(chip,TDIPin);
    TCK = gpiod_chip_get_line(chip,TCKPin);
    TDO = gpiod_chip_get_line(chip,TDOPin);

      // Open LED lines for output
    gpiod_line_request_output(TMS, "xc3sprog",0);
    gpiod_line_request_output(TDI, "xc3sprog",0);
    gpiod_line_request_output(TCK, "xc3sprog",0);

    // Open switch line for input
    gpiod_line_request_input(TDO, "xc3sprog");

}

IOSysGPIOD ::~IOSysGPIOD ()
{
  gpiod_line_release(TMS);
  gpiod_line_release(TDI);
  gpiod_line_release(TCK);
  gpiod_line_release(TDO);
  gpiod_chip_close(chip);
}

void IOSysGPIOD::txrx_block(const unsigned char *tdi, unsigned char *tdo,
                             int length, bool last) {
  int i = 0;
  int j = 0;
  unsigned char tdo_byte = 0;
  unsigned char tdi_byte;

  if (tdi) tdi_byte = tdi[j];

  while (i < length - 1) {
    tdo_byte = tdo_byte + (txrx(false, (tdi_byte & 1) == 1) << (i % 8));
    if (tdi) tdi_byte = tdi_byte >> 1;
    i++;
    if ((i % 8) == 0) {            // Next byte
      if (tdo) tdo[j] = tdo_byte;  // Save the TDO byte
      tdo_byte = 0;
      j++;
      if (tdi) tdi_byte = tdi[j];  // Get the next TDI byte
    }
  };
  tdo_byte = tdo_byte + (txrx(last, (tdi_byte & 1) == 1) << (i % 8));
  if (tdo) tdo[j] = tdo_byte;

  gpiod_line_set_value(TCK,0);

  return;
}

void IOSysGPIOD::tx_tms(unsigned char *pat, int length, int force) {
  int i;
  unsigned char tms;
  for (i = 0; i < length; i++) {
    if ((i & 0x7) == 0) tms = pat[i >> 3];
    tx((tms & 0x01), true);
    tms = tms >> 1;
  }

  gpiod_line_set_value(TCK,0);
}

void IOSysGPIOD::tx(bool tms, bool tdi) {
  gpiod_line_set_value(TCK,1);

    if(tdi)
      gpiod_line_set_value(TDI, 1);
   else
      gpiod_line_set_value(TDI, 0);

   if(tms)
      gpiod_line_set_value(TMS, 1);
   else
      gpiod_line_set_value(TMS, 0);

  gpiod_line_set_value(TCK,0);
}

bool IOSysGPIOD::txrx(bool tms, bool tdi) {
  static char buf[1];

  tx(tms, tdi);

  return gpiod_line_get_value(TDO);
}
