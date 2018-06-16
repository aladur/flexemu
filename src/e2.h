/*
    e2.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


/***************************************************
*  specifying video display geometry of Eurocom II *
****************************************************

 only the first three values should be adapted
 the other values depend on them
*/

#ifndef __e2_h__
#define __e2_h__

#define VIDEORAM_SIZE   0x4000
#define RASTERLINE_SIZE 64
#define YBLOCK_BASE 4   /* number of yblocks as a power of 2 */
#define COLOR_PLANES    6   /* number of color planes */
#define MAXVIDEORAM_BANKS (48)  /* max number of ram banks of size 16K   */
/* possible values: 12, 48 */
/* number of yblocks */
#define YBLOCKS     (1 << YBLOCK_BASE) /* Nr. of blocks verticaly */
/* bytesize of one yblock */
#define YBLOCK_SIZE (VIDEORAM_SIZE / YBLOCKS)

/* pixelsize of one block */
#define BLOCKWIDTH  (RASTERLINE_SIZE << 3)
#define BLOCKHEIGHT (YBLOCK_SIZE / RASTERLINE_SIZE)

/* pixelsize of whole video display represented by a window */
#define WINDOWWIDTH (RASTERLINE_SIZE << 3)
#define WINDOWHEIGHT    (VIDEORAM_SIZE / RASTERLINE_SIZE)

/***************************************************
*  specifying memory mapped I/O                    *
***************************************************/

#define ONE_BYTE    (1)
#define TWO_BYTE    (2)
#define FOUR_BYTE   (4)
#define SIXTEEN_BYTE    (16)

#define VIRAM_MASK  (~(VIDEORAM_SIZE - 1) & 0xffff) /* Video ram */
#define VIRAM_BASE  0x0000
/* IOGEN_MASK and IOIGEN_BASE provides a general address range */
/* where memory mapped I/O is placed                           */
/* the range is: $fc00 - ffff                                  */
#define GENIO_MASK  0xfc00   /* general mask for I/O (One 1K Block) */
#define GENIO_BASE  0xfc00
#define ACIA1_MASK  TWO_BYTE /* uart */
#define ACIA1_BASE  0xfcf4
#define VICO_BASE   0xfcf6
#define VICO_MASK   TWO_BYTE /* Video control registers */
#define ACIA1_MASK  TWO_BYTE /* serial I/O, terminal driver */
#define ACIA1_BASE  0xfcf4
#define VICO_BASE   0xfcf6
#define PIA1_MASK   FOUR_BYTE /* parallel I/O, keyboard driver */
#define PIA1_BASE   0xfcf0
#define PIA2_MASK   FOUR_BYTE /* parallel I/O, Joystick, beep */
#define PIA2_BASE   0xfcf8
#define FDC_MASK   FOUR_BYTE /* Floppy disk controller WD1793 */
#define FDC_BASE   0xfd30
#define DRISEL_MASK   ONE_BYTE /* Floppy disk controller drive select */
#define DRISEL_BASE   0xfd38
#define COMM_MASK   ONE_BYTE /* command device, only for emulation */
#define COMM_BASE   0xfd3c
#define MMU_MASK    SIXTEEN_BYTE /* Memory management unit */
#define MMU_BASE    0xffe0
#define ROM_BASE    0xf000   /* Start addr. of ROM up to 0xffff */
#define RTC_LOW     0xfd00   /* start addr. of realtime clock */
#define RTC_HIGH    0xfd2f   /* last addr. of realtime clock */

/******************************************************
*  Flag for configuring Monitor I/O to parallel or    *
*  serial Input/Output. Can be used to switch         *
*  between Graphics View (parallel) and terminal view *
*  (serial)                                           *
*******************************************************/

#define SERPAR      (0xef86)

/******************************************************
* initial value for SERPAR after RESET or NMI         *
******************************************************/

#define INITSP      (0xf07c)
#endif

