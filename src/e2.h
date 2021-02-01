/*
    e2.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2021  W. Schwotzer

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

#ifndef E2_INCLUDED
#define E2_INCLUDED

#define VIDEORAM_SIZE   0x4000
#define RASTERLINE_SIZE 64
#define YBLOCK_BASE 4   /* number of yblocks as a power of 2 */
#define COLOR_PLANES    6   /* number of color planes */
#define MAX_COLORS (1U << COLOR_PLANES)   /* maximum number of color values */
#define RED_HIGH 0x10 /* Color bitmask for green high */
#define RED_LOW 0x02 /* Color bitmask for red low */
#define GREEN_HIGH 0x20 /* Color bitmask for green high */
#define GREEN_LOW 0x04 /* Color bitmask for green low */
#define BLUE_HIGH 0x08 /* Color bitmask for blue low */
#define BLUE_LOW 0x01 /* Color bitmask for blue low */
#define MAXVIDEORAM_BANKS (48u)  /* max number of ram banks of size 16K   */
/* possible values: 12, 48 */
/* number of yblocks */
#define YBLOCKS     (1 << YBLOCK_BASE) /* Nr. of blocks vertically */
/* bytesize of one yblock */
#define YBLOCK_SIZE (VIDEORAM_SIZE / YBLOCKS)

/* pixelsize of one block */
#define BLOCKWIDTH  (RASTERLINE_SIZE << 3)
#define BLOCKHEIGHT (YBLOCK_SIZE / RASTERLINE_SIZE)

/* pixelsize of whole video display represented by a window */
#define WINDOWWIDTH (RASTERLINE_SIZE << 3)
#define WINDOWHEIGHT    (VIDEORAM_SIZE / RASTERLINE_SIZE)

/* GENIO_BASE provides a general address range  */
/* where memory mapped I/O is placed            */
/* the range is: $fc00 - ffff                   */
/* It has to be a multiple of 1024              */
#define GENIO_BASE  0xfc00
#define ROM_BASE    0xf000   /* Start addr. of ROM up to 0xffff */

#define ORIGINAL_FREQUENCY 1.3396f
#define ORIGINAL_PERIOD (1.0f / ORIGINAL_FREQUENCY)

#endif

