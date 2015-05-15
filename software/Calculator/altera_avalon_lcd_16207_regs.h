#ifndef __ALTERA_AVALON_LCD_16207_REGS_H__
#define __ALTERA_AVALON_LCD_16207_REGS_H__

/*
///////////////////////////////////////////////////////////////////////////
//
// ALTERA_AVALON_LCD_16207 PERIPHERAL
//
// Provides a hardware interface that allows software to
// access the two (2) internal 8-bit registers in an Optrex
// model 16207 (or equivalent) character LCD display (the kind
// shipped with the Nios Development Kit, 2 rows x 16 columns).
//
// Because the interface to the LCD module is "not quite Avalon,"
// the hardware in this module ends-up mapping the module's
// two physical read-write registers into four Avalon-visible
// registers:  Two read-only registers and two write-only registers.
// A picture is worth a thousand words:
//
// THE REGISTER MAP
//
//              7     6     5     4     3     2     1     0     Offset
//           +-----+-----+-----+-----+-----+-----+-----+-----+
// RS = 0    |         Command Register (WRITE-Only)         |  0
//           +-----+-----+-----+-----+-----+-----+-----+-----+
// RS = 0    |         Status Register  (READ -Only)         |  1
//           +-----+-----+-----+-----+-----+-----+-----+-----+
// RS = 1    |         Data Register    (WRITE-Only)         |  2
//           +-----+-----+-----+-----+-----+-----+-----+-----+
// RS = 1    |         Data Register    (READ -Only)         |  3
//           +-----+-----+-----+-----+-----+-----+-----+-----+
//
///////////////////////////////////////////////////////////////////////////
*/

#include <io.h>

#define IOADDR_ALTERA_AVALON_LCD_16207_COMMAND(base)      __IO_CALC_ADDRESS_NATIVE(base, 0)
#define IOWR_ALTERA_AVALON_LCD_16207_COMMAND(base, data)  IOWR(base, 0, data)

#define IOADDR_ALTERA_AVALON_LCD_16207_STATUS(base)       __IO_CALC_ADDRESS_NATIVE(base, 1)
#define IORD_ALTERA_AVALON_LCD_16207_STATUS(base)         IORD(base, 1)

#define ALTERA_AVALON_LCD_16207_STATUS_BUSY_MSK           (0x00000080u)
#define ALTERA_AVALON_LCD_16207_STATUS_BUSY_OFST          (7)

#define IOADDR_ALTERA_AVALON_LCD_16207_DATA_WR(base)      __IO_CALC_ADDRESS_NATIVE(base, 2)
#define IOWR_ALTERA_AVALON_LCD_16207_DATA(base, data)     IOWR(base, 2, data)

#define IOADDR_ALTERA_AVALON_LCD_16207_DATA_RD(base)      __IO_CALC_ADDRESS_NATIVE(base, 3)
#define IORD_ALTERA_AVALON_LCD_16207_DATA(base)           IORD(base, 3)

#endif
