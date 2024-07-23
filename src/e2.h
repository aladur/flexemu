/*
    e2.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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

#include <cstdint>


const unsigned VIDEORAM_SIZE{0x4000U};

enum : uint8_t {
RASTERLINE_SIZE = 64, /* byte size of one raster-line */
};

enum : uint8_t {
YBLOCK_BASE = 4, /* number of yblocks as a power of 2 */
};

enum : uint8_t {
COLOR_PLANES = 6,   /* maximum number of color planes */
};

enum : uint8_t {
MAX_COLORS = (1U << COLOR_PLANES),   /* maximum number of color values */
};

enum : uint8_t {
SCREEN_SIZES = 5, /* maximum screen size factor */
};

enum : uint8_t {
ICON_SIZES = 3, /* maximum icon size factor */
};

enum : uint8_t {
RED_HIGH = 0x10, /* Color bitmask for green high */
RED_LOW = 0x02, /* Color bitmask for red low */
GREEN_HIGH = 0x20, /* Color bitmask for green high */
GREEN_LOW = 0x04, /* Color bitmask for green low */
BLUE_HIGH = 0x08, /* Color bitmask for blue low */
BLUE_LOW = 0x01, /* Color bitmask for blue low */
};

const unsigned MAXVIDEORAM_BANKS{48U}; /* max number of ram banks of size 16K */

/* possible values: 12, 48 */
/* number of yblocks */
enum : uint8_t {
YBLOCKS = (1U << YBLOCK_BASE), /* Nr. of blocks vertically */
};

/* bytesize of one yblock */
enum : uint16_t {
YBLOCK_SIZE = (VIDEORAM_SIZE / YBLOCKS),
};

/* pixelsize of one block */
enum : uint16_t {
BLOCKWIDTH = (RASTERLINE_SIZE << 3U),
BLOCKHEIGHT = (YBLOCK_SIZE / RASTERLINE_SIZE),
};

/* pixelsize of whole video display represented by a window */
enum : uint16_t {
WINDOWWIDTH = (RASTERLINE_SIZE << 3U),
WINDOWHEIGHT = (VIDEORAM_SIZE / RASTERLINE_SIZE),
};

/* GENIO_BASE provides a general address range  */
/* where memory mapped I/O is placed            */
/* the range is: $fc00 - ffff                   */
/* It has to be a multiple of 1024              */
/* ROM_BASE defines the base address where ROM  */
/* memory is locatad (read-only memory).        */
enum : uint16_t {
GENIO_BASE = 0xfc00, /* Start addr. of mm-I/O up to 0xffff */
ROM_BASE = 0xf000, /* Start addr. of ROM up to 0xffff */
};

const uint8_t MAX_DRIVES = 4; /* Max. number of supported disk drives */

/* The default CPU frequncy [MHz] and time period [micro-seconds] */
constexpr float ORIGINAL_FREQUENCY = 1.3396F;
constexpr float ORIGINAL_PERIOD = (1.0F / ORIGINAL_FREQUENCY);

#endif

