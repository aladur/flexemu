/*
    bmp.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2022  W. Schwotzer

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



#ifndef BMP_INCLUDED
#define BMP_INCLUDED

#include "typedefs.h"

/**
 * Windows Bitmap (*.bmp) file format definitions.
 * All data values have little endian format.
 *
 * For details see:
 * https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader
 * https://docs.microsoft.com/en-us/previous-versions//dd183376(v=vs.85)
 * https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-rgbquad
 * https://en.wikipedia.org/wiki/BMP_file_format
 * Hint for static_assert inspired by Dan Saks
 */

#pragma pack(push, 1)
struct sBITMAPFILEHEADER
{
    Byte type[2];
    DWord fileSize;
    Word reserved[2];
    DWord dataOffset;
};
static_assert(sizeof(sBITMAPFILEHEADER) == 14, "sBITMAPFILEHEADER is padded");

struct sBITMAPINFOHEADER
{
    DWord size;
    SDWord width;
    SDWord height;
    Word planes;
    Word bitCount;
    DWord compression;
    DWord imageSize;
    SDWord xPixelsPerMeter;
    SDWord yPixelsPerMeter;
    DWord colorsUsed;
    DWord colorsImportant;
};
static_assert(sizeof(sBITMAPINFOHEADER) == 40, "sBITMAPINFOHEADER is padded");

struct sRGBQUAD
{
    Byte blue;
    Byte green;
    Byte red;
    Byte reserved;
};
static_assert(sizeof(sRGBQUAD) == 4, "sRGBQUAD is padded");
#pragma pack(pop)

/* supported compression values */
#ifndef BI_RGB
#define BI_RGB 0U
#endif

#ifndef BI_RLE8
#define BI_RLE8 1U
#endif

#ifndef BI_RLE4
#define BI_RLE4 2U
#endif


#endif

