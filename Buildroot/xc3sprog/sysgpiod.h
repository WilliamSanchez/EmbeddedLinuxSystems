#ifndef _IO_SYSGPIOD_
#define _IO_SYSGPIOD_

#include "iobase.h"

class IOSysGPIOD : public IOBase 
{
    public:
        IOSysGPIOD (int tms, int tck, int tdi, int tdo);
        virtual ~IOSysGPIOD ();

    protected:
        void tx(bool tms, bool tdi);
        bool txrx(bool tms, bool tdi);

        void txrx_block(const unsigned char *tdi, unsigned char *tdo, int length, bool last);
        void tx_tms(unsigned char *pat, int length, int force);

    private:
        int TMSPin;
        int TCKPin;
        int TDIPin;
        int TDOPin;

        const char *chipname;
            struct gpiod_chip *chip;
    struct gpiod_line *TMS;
    struct gpiod_line *TDI;
    struct gpiod_line *TCK;
    struct gpiod_line *TDO;
};





#endif