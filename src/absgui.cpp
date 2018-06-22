/*
    absgui.cc: abstract graphical user interface


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

#include <sstream>
#include <iomanip>
#include "misc1.h"
#include "pia1.h"
#include "absgui.h"
#include "vico1.h"
#include "vico2.h"
#include "mc6809.h"
#include "memory.h"
#include "inout.h"
#include "schedule.h"
#include "mc6809st.h"
#include "joystick.h"
#include "keyboard.h"
#include "terminal.h"

Word internal_c2bt[8] =
{
#ifdef WORDS_BIGENDIAN
    0x0003, 0x000C, 0x0030, 0x00C0,
    0x0300, 0x0C00, 0x3000, 0xC000
#else
    0x0300, 0x0C00, 0x3000, 0xC000,
    0x0003, 0x000C, 0x0030, 0x00C0
#endif
};

DWord internal_c3bt[8] =
{
#if 1
    0x000007, 0x000038, 0x0001C0, 0x000E00,
    0x007000, 0x038000, 0x1C0000, 0xE00000
#else
    0x070000, 0x380000, 0xC00100, 0x000E00,
    0x007000, 0x008003, 0x00001C, 0x0000E0
#endif
};

DWord internal_c4bt[8] =
{
#ifdef WORDS_BIGENDIAN
    0x0000000F, 0x000000F0, 0x00000F00, 0x0000F000,
    0x000F0000, 0x00F00000, 0x0F000000, 0xF0000000
#else
    0x0F000000, 0xF0000000, 0x000F0000, 0x00F00000,
    0x00000F00, 0x0000F000, 0x0000000F, 0x000000F0
#endif
};

void AbstractGui::update_block(int)
{
}

void AbstractGui::initialize(struct sGuiOptions &options)
{
    switch_sp = 0;
    program_name = "";
    timebase = 20; // timer event every x milliseconds

    program_name    = options.argv[0];
    switch_sp   = options.switch_sp;
    pixelSizeX    = options.pixelSizeX;
    pixelSizeY    = options.pixelSizeY;
    nColors     = options.nColors;
    withColorScale  = !stricmp(options.color.c_str(), "default");
    color       = options.color;

}

void AbstractGui::initialize_conv_tables()
{
    int i, j;

    for (i = 0; i < 256; i++)
    {
        conv_2byte_tab[i] = 0;
        conv_3byte_tab[i] = 0;
        conv_4byte_tab[i] = 0;

        for (j = 0; j < 8; j++)
        {
            if (i & (1 << j))
            {
                conv_2byte_tab[i] |= internal_c2bt[j] ;
                conv_3byte_tab[i] |= internal_c3bt[j] ;
                conv_4byte_tab[i] |= internal_c4bt[j] ;
            }
        } // for
    } // for

    // the default color table
    for (i = 0; i < (1 << COLOR_PLANES); i++)
    {
        pens[i] = i;
    }
} // initialize_conv_tables

void AbstractGui::main_loop()
{
    // to be implemented by subclass
}

void AbstractGui::update_cpuview(const Mc6809CpuStatus &stat)
{
    redraw_cpuview(stat);
}

/*
 * CopyToZPixmap
 * Copy part of the video RAM into a Z-pixel-map.
 * dest    read-write Pointer into result Z-pixel-map.
 * video_ram read-only Pointer into part of video RAM.
 * depth   Color depth of Z-pixel-map, supported values: { 8, 16, 24, 32 }
 */
void AbstractGui::CopyToZPixmap(const Byte *dest, Byte const *video_ram,
                                int depth)
{
    int count;              /* Byte counter into video RAM          */
    Byte pixels[6]; /* One byte of video RAM for each plane */

    memset(pixels, 0, sizeof(pixels));

    for (count = 0; count < YBLOCK_SIZE; ++count)
    {
        Byte pixelBitMask;
        bool isEndOfRasterLine = false;
        if ((count % RASTERLINE_SIZE) == (RASTERLINE_SIZE - 1) &&
            pixelSizeY > 1)
        {
            isEndOfRasterLine = true;
        }

        if (video_ram != nullptr)
        {

            pixels[0] = video_ram[0];

            if (nColors > 2)
            {
                pixels[2] = video_ram[VIDEORAM_SIZE];
                pixels[4] = video_ram[VIDEORAM_SIZE * 2];

                if (nColors > 8)
                {
                    pixels[1] = video_ram[VIDEORAM_SIZE * 3];
                    pixels[3] = video_ram[VIDEORAM_SIZE * 4];
                    pixels[5] = video_ram[VIDEORAM_SIZE * 5];
                }
            }

            video_ram++;
        }

        /* Use MSBit first */
        for (pixelBitMask = 128; pixelBitMask; pixelBitMask >>= 1)
        {
            unsigned int penIndex = 0; /* calculated pen index */

            if (video_ram != nullptr)
            {
                if (pixels[0] & pixelBitMask)
                {
                    penIndex |= 32;    // 0x0C, green high
                }

                if (nColors > 8)
                {
                    if (pixels[2] & pixelBitMask)
                    {
                        penIndex |= 16;    // 0x0D, red high
                    }

                    if (pixels[4] & pixelBitMask)
                    {
                        penIndex |= 8;    // 0x0E, blue high
                    }

                    if (pixels[1] & pixelBitMask)
                    {
                        penIndex |= 4;    // 0x04, green low
                    }

                    if (pixels[3] & pixelBitMask)
                    {
                        penIndex |= 2;    // 0x05, red low
                    }

                    if (pixels[5] & pixelBitMask)
                    {
                        penIndex |= 1;    // 0x06, blue low
                    }
                }
                else
                {
                    if (pixels[2] & pixelBitMask)
                    {
                        penIndex |= 8;    // 0x0D, red high
                    }

                    if (pixels[4] & pixelBitMask)
                    {
                        penIndex |= 2;    // 0x0E, blue high
                    }
                }
            }
            else
            {
                // If no source is available use highest available color
                penIndex = options.isInverse ? 0 : 63;
            }

            if (depth == 32 || depth == 24)
            {
                DWord *dest32 = (DWord *)dest;
                DWord pen = (DWord)pens[penIndex];

                switch (pixelSizeX << 4 | pixelSizeY)
                {
                    case 0x11:
                        *(dest32++) = pen;
                        break;

                    case 0x12:
                        *(dest32) = pen;
                        *(dest32 + WINDOWWIDTH) = pen;
                        dest32++;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest32 += WINDOWWIDTH;
                        }

                        break;

                    case 0x13:
                        *(dest32) = pen;
                        *(dest32 + WINDOWWIDTH) = pen;
                        *(dest32 + 2 * WINDOWWIDTH) = pen;
                        dest32++;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest32 += 2 * WINDOWWIDTH;
                        }

                        break;

                    case 0x14:
                        *(dest32) = pen;
                        *(dest32 + WINDOWWIDTH) = pen;
                        *(dest32 + 2 * WINDOWWIDTH) = pen;
                        *(dest32 + 3 * WINDOWWIDTH) = pen;
                        dest32++;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest32 += 3 * WINDOWWIDTH;
                        }

                        break;

                    case 0x21:
                        *(dest32++) = pen;
                        *(dest32++) = pen;
                        break;

                    case 0x22:
                        *(dest32) = pen;
                        *(dest32 + 1) = pen;
                        *(dest32 + 2 * WINDOWWIDTH) = pen;
                        *(dest32 + 2 * WINDOWWIDTH + 1) = pen;
                        dest32 += 2;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest32 += 2 * WINDOWWIDTH;
                        }

                        break;

                    case 0x23:
                        *(dest32) = pen;
                        *(dest32 + 1) = pen;
                        *(dest32 + 2 * WINDOWWIDTH) = pen;
                        *(dest32 + 2 * WINDOWWIDTH + 1) = pen;
                        *(dest32 + 4 * WINDOWWIDTH) = pen;
                        *(dest32 + 4 * WINDOWWIDTH + 1) = pen;
                        dest32 += 2;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest32 += 4 * WINDOWWIDTH;
                        }

                        break;

                    case 0x24:
                        *(dest32) = pen;
                        *(dest32 + 1) = pen;
                        *(dest32 + 2 * WINDOWWIDTH) = pen;
                        *(dest32 + 2 * WINDOWWIDTH + 1) = pen;
                        *(dest32 + 4 * WINDOWWIDTH) = pen;
                        *(dest32 + 4 * WINDOWWIDTH + 1) = pen;
                        *(dest32 + 6 * WINDOWWIDTH) = pen;
                        *(dest32 + 6 * WINDOWWIDTH + 1) = pen;
                        dest32 += 2;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest32 += 6 * WINDOWWIDTH;
                        }

                        break;

                    case 0x31:
                        *(dest32++) = pen;
                        *(dest32++) = pen;
                        *(dest32++) = pen;
                        break;

                    case 0x32:
                        *(dest32) = pen;
                        *(dest32 + 1) = pen;
                        *(dest32 + 2) = pen;
                        *(dest32 + 3 * WINDOWWIDTH) = pen;
                        *(dest32 + 3 * WINDOWWIDTH + 1) = pen;
                        *(dest32 + 3 * WINDOWWIDTH + 2) = pen;
                        dest32 += 3;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest32 += 3 * WINDOWWIDTH;
                        }

                        break;

                    case 0x33:
                        *(dest32) = pen;
                        *(dest32 + 1) = pen;
                        *(dest32 + 2) = pen;
                        *(dest32 + 3 * WINDOWWIDTH) = pen;
                        *(dest32 + 3 * WINDOWWIDTH + 1) = pen;
                        *(dest32 + 3 * WINDOWWIDTH + 2) = pen;
                        *(dest32 + 6 * WINDOWWIDTH) = pen;
                        *(dest32 + 6 * WINDOWWIDTH + 1) = pen;
                        *(dest32 + 6 * WINDOWWIDTH + 2) = pen;
                        dest32 += 3;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest32 += 6 * WINDOWWIDTH;
                        }

                        break;

                    case 0x34:
                        *(dest32) = pen;
                        *(dest32 + 1) = pen;
                        *(dest32 + 2) = pen;
                        *(dest32 + 3 * WINDOWWIDTH) = pen;
                        *(dest32 + 3 * WINDOWWIDTH + 1) = pen;
                        *(dest32 + 3 * WINDOWWIDTH + 2) = pen;
                        *(dest32 + 6 * WINDOWWIDTH) = pen;
                        *(dest32 + 6 * WINDOWWIDTH + 1) = pen;
                        *(dest32 + 6 * WINDOWWIDTH + 2) = pen;
                        *(dest32 + 9 * WINDOWWIDTH) = pen;
                        *(dest32 + 9 * WINDOWWIDTH + 1) = pen;
                        *(dest32 + 9 * WINDOWWIDTH + 2) = pen;
                        dest32 += 3;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest32 += 9 * WINDOWWIDTH;
                        }

                        break;
                }
                dest = (Byte *)dest32;
            }
            else if (depth == 16 || depth == 15)
            {
                Word *dest16 = (Word *)dest;
                Word pen = (Word)pens[penIndex];

                switch (pixelSizeX << 4 | pixelSizeY)
                {
                    case 0x11:
                        *(dest16++) = pen;
                        break;

                    case 0x12:
                        *(dest16) = pen;
                        *(dest16 + WINDOWWIDTH) = pen;
                        dest16++;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest16 += WINDOWWIDTH;
                        }

                        break;

                    case 0x13:
                        *(dest16) = pen;
                        *(dest16 + WINDOWWIDTH) = pen;
                        *(dest16 + 2 * WINDOWWIDTH) = pen;
                        dest16++;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest16 += 2 * WINDOWWIDTH;
                        }

                        break;

                    case 0x14:
                        *(dest16) = pen;
                        *(dest16 + WINDOWWIDTH) = pen;
                        *(dest16 + 2 * WINDOWWIDTH) = pen;
                        *(dest16 + 3 * WINDOWWIDTH) = pen;
                        dest16++;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest16 += 3 * WINDOWWIDTH;
                        }

                        break;

                    case 0x21:
                        *(dest16++) = pen;
                        *(dest16++) = pen;
                        break;

                    case 0x22:
                        *(dest16) = pen;
                        *(dest16 + 1) = pen;
                        *(dest16 + 2 * WINDOWWIDTH) = pen;
                        *(dest16 + 2 * WINDOWWIDTH + 1) = pen;
                        dest16 += 2;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest16 += 2 * WINDOWWIDTH;
                        }

                        break;

                    case 0x23:
                        *(dest16) = pen;
                        *(dest16 + 1) = pen;
                        *(dest16 + 2 * WINDOWWIDTH) = pen;
                        *(dest16 + 2 * WINDOWWIDTH + 1) = pen;
                        *(dest16 + 4 * WINDOWWIDTH) = pen;
                        *(dest16 + 4 * WINDOWWIDTH + 1) = pen;
                        dest16 += 2;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest16 += 4 * WINDOWWIDTH;
                        }

                        break;

                    case 0x24:
                        *(dest16) = pen;
                        *(dest16 + 1) = pen;
                        *(dest16 + 2 * WINDOWWIDTH) = pen;
                        *(dest16 + 2 * WINDOWWIDTH + 1) = pen;
                        *(dest16 + 4 * WINDOWWIDTH) = pen;
                        *(dest16 + 4 * WINDOWWIDTH + 1) = pen;
                        *(dest16 + 6 * WINDOWWIDTH) = pen;
                        *(dest16 + 6 * WINDOWWIDTH + 1) = pen;
                        dest16 += 2;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest16 += 6 * WINDOWWIDTH;
                        }

                        break;

                    case 0x31:
                        *(dest16++) = pen;
                        *(dest16++) = pen;
                        *(dest16++) = pen;
                        break;

                    case 0x32:
                        *(dest16) = pen;
                        *(dest16 + 1) = pen;
                        *(dest16 + 2) = pen;
                        *(dest16 + 3 * WINDOWWIDTH) = pen;
                        *(dest16 + 3 * WINDOWWIDTH + 1) = pen;
                        *(dest16 + 3 * WINDOWWIDTH + 2) = pen;
                        dest16 += 3;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest16 += 3 * WINDOWWIDTH;
                        }

                        break;

                    case 0x33:
                        *(dest16) = pen;
                        *(dest16 + 1) = pen;
                        *(dest16 + 2) = pen;
                        *(dest16 + 3 * WINDOWWIDTH) = pen;
                        *(dest16 + 3 * WINDOWWIDTH + 1) = pen;
                        *(dest16 + 3 * WINDOWWIDTH + 2) = pen;
                        *(dest16 + 6 * WINDOWWIDTH) = pen;
                        *(dest16 + 6 * WINDOWWIDTH + 1) = pen;
                        *(dest16 + 6 * WINDOWWIDTH + 2) = pen;
                        dest16 += 3;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest16 += 6 * WINDOWWIDTH;
                        }

                        break;

                    case 0x34:
                        *(dest16) = pen;
                        *(dest16 + 1) = pen;
                        *(dest16 + 2) = pen;
                        *(dest16 + 3 * WINDOWWIDTH) = pen;
                        *(dest16 + 3 * WINDOWWIDTH + 1) = pen;
                        *(dest16 + 3 * WINDOWWIDTH + 2) = pen;
                        *(dest16 + 6 * WINDOWWIDTH) = pen;
                        *(dest16 + 6 * WINDOWWIDTH + 1) = pen;
                        *(dest16 + 6 * WINDOWWIDTH + 2) = pen;
                        *(dest16 + 9 * WINDOWWIDTH) = pen;
                        *(dest16 + 9 * WINDOWWIDTH + 1) = pen;
                        *(dest16 + 9 * WINDOWWIDTH + 2) = pen;
                        dest16 += 3;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest16 += 9 * WINDOWWIDTH;
                        }

                        break;
                }
                dest = (Byte *)dest16;
            }
            else if (depth == 8 || depth == 1)
            {
                Byte *dest8 = (Byte *)dest;
                Byte pen = (Byte)pens[penIndex];

                switch (pixelSizeX << 4 | pixelSizeY)
                {
                    case 0x11:
                        *(dest8++) = pen;
                        break;

                    case 0x12:
                        *(dest8) = pen;
                        *(dest8 + WINDOWWIDTH) = pen;
                        dest8++;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest8 += WINDOWWIDTH;
                        }

                        break;

                    case 0x13:
                        *(dest8) = pen;
                        *(dest8 + WINDOWWIDTH) = pen;
                        *(dest8 + 2 * WINDOWWIDTH) = pen;
                        dest8++;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest8 += 2 * WINDOWWIDTH;
                        }

                        break;

                    case 0x14:
                        *(dest8) = pen;
                        *(dest8 + WINDOWWIDTH) = pen;
                        *(dest8 + 2 * WINDOWWIDTH) = pen;
                        *(dest8 + 3 * WINDOWWIDTH) = pen;
                        dest8++;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest8 += 3 * WINDOWWIDTH;
                        }

                        break;

                    case 0x21:
                        *(dest8++) = pen;
                        *(dest8++) = pen;
                        break;

                    case 0x22:
                        *(dest8) = pen;
                        *(dest8 + 1) = pen;
                        *(dest8 + 2 * WINDOWWIDTH) = pen;
                        *(dest8 + 2 * WINDOWWIDTH + 1) = pen;
                        dest8 += 2;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest8 += 2 * WINDOWWIDTH;
                        }

                        break;

                    case 0x23:
                        *(dest8) = pen;
                        *(dest8 + 1) = pen;
                        *(dest8 + 2 * WINDOWWIDTH) = pen;
                        *(dest8 + 2 * WINDOWWIDTH + 1) = pen;
                        *(dest8 + 4 * WINDOWWIDTH) = pen;
                        *(dest8 + 4 * WINDOWWIDTH + 1) = pen;
                        dest8 += 2;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest8 += 4 * WINDOWWIDTH;
                        }

                        break;

                    case 0x24:
                        *(dest8) = pen;
                        *(dest8 + 1) = pen;
                        *(dest8 + 2 * WINDOWWIDTH) = pen;
                        *(dest8 + 2 * WINDOWWIDTH + 1) = pen;
                        *(dest8 + 4 * WINDOWWIDTH) = pen;
                        *(dest8 + 4 * WINDOWWIDTH + 1) = pen;
                        *(dest8 + 6 * WINDOWWIDTH) = pen;
                        *(dest8 + 6 * WINDOWWIDTH + 1) = pen;
                        dest8 += 2;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest8 += 6 * WINDOWWIDTH;
                        }

                        break;

                    case 0x31:
                        *(dest8) = pen;
                        *(dest8 + 1) = pen;
                        *(dest8 + 2) = pen;
                        dest8 += 3;
                        break;

                    case 0x32:
                        *(dest8) = pen;
                        *(dest8 + 1) = pen;
                        *(dest8 + 2) = pen;
                        *(dest8 + 3 * WINDOWWIDTH) = pen;
                        *(dest8 + 3 * WINDOWWIDTH + 1) = pen;
                        *(dest8 + 3 * WINDOWWIDTH + 2) = pen;
                        dest8 += 3;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest8 += 3 * WINDOWWIDTH;
                        }

                        break;

                    case 0x33:
                        *(dest8) = pen;
                        *(dest8 + 1) = pen;
                        *(dest8 + 2) = pen;
                        *(dest8 + 3 * WINDOWWIDTH) = pen;
                        *(dest8 + 3 * WINDOWWIDTH + 1) = pen;
                        *(dest8 + 3 * WINDOWWIDTH + 2) = pen;
                        *(dest8 + 6 * WINDOWWIDTH) = pen;
                        *(dest8 + 6 * WINDOWWIDTH + 1) = pen;
                        *(dest8 + 6 * WINDOWWIDTH + 2) = pen;
                        dest8 += 3;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest8 += 6 * WINDOWWIDTH;
                        }

                        break;

                    case 0x34:
                        *(dest8) = pen;
                        *(dest8 + 1) = pen;
                        *(dest8 + 2) = pen;
                        *(dest8 + 3 * WINDOWWIDTH) = pen;
                        *(dest8 + 3 * WINDOWWIDTH + 1) = pen;
                        *(dest8 + 3 * WINDOWWIDTH + 2) = pen;
                        *(dest8 + 6 * WINDOWWIDTH) = pen;
                        *(dest8 + 6 * WINDOWWIDTH + 1) = pen;
                        *(dest8 + 6 * WINDOWWIDTH + 2) = pen;
                        *(dest8 + 9 * WINDOWWIDTH) = pen;
                        *(dest8 + 9 * WINDOWWIDTH + 1) = pen;
                        *(dest8 + 9 * WINDOWWIDTH + 2) = pen;
                        dest8 += 3;

                        if (isEndOfRasterLine && pixelBitMask== 1)
                        {
                            dest8 += 9 * WINDOWWIDTH;
                        }

                        break;
                }
                dest = (Byte *)dest8;
            }
            else
            {
                printf("Color depth=%d is not supported\n", depth);
            }
        }
    }
}

void AbstractGui::redraw_cpuview(const Mc6809CpuStatus &stat)
{
    int i;

    clear_cpuview();
    text(0, 0, "Cycl:");
    text(0, 1, "Inst:");
    text(0, 2, "  PC:");
    text(0, 3, "   S:");
    text(0, 4, "   U:");
    text(0, 5, "   X:");
    text(0, 6, "   Y:");

    text(15, 2, "EFHINZVC");
    text(10, 3, "  CC:");
    text(10, 4, "  DP:");
    text(10, 5, "   A:");
    text(10, 6, "   B:");
    text(19, 5, "  bp1:");
    text(19, 6, "  bp2:");
    text(22, 0, "Freq:");
    text(35, 0, "MHz");

    for (i = 0; i < 6; ++i)
    {
        text(4, i + 8, ":");
    }

    redraw_cpuview_contents(stat);
}

void AbstractGui::redraw_cpuview_contents(const Mc6809CpuStatus &stat)
{
    static char             tmp[40];
    Byte                    i, j;
    Word            mem_addr;
    std::stringstream total_cycles;

    mem_addr = ((stat.s >> 3) << 3) - 16;
    text(6, 2, hexstr(stat.pc));
    text(6, 3, hexstr(stat.s));
    text(6, 4, hexstr(stat.u));
    text(6, 5, hexstr(stat.x));
    text(6, 6, hexstr(stat.y));
    text(15, 3, binstr(stat.cc));
    text(15, 4, hexstr(stat.dp));
    text(15, 5, hexstr(stat.a));
    text(15, 6, hexstr(stat.b));
    total_cycles << std::setw(16) << stat.total_cycles;

    text(5,  0, total_cycles.str().c_str());
    sprintf(tmp, "%6.2f", stat.freq);
    text(28,  0, tmp);
    text(6, 1, "                        "); // first clear area

    if (stat.mnemonic[0] != '\0')
    {
        text(6, 1, stat.mnemonic);
    }
    else
    {
        text(6, 1, "sorry, no disassembler installed");
    }

    for (i = 0; i < 2; i++)
    {
        if (!cpu.is_bp_set(i))
        {
            text(25, 5 + i, "    ", bp_input[i]);
        }
        else
            text(25, 5 + i,
                 hexstr((Word)cpu.get_bp(i)),
                 bp_input[i]);
    }  // for

    Word stk = 0;
    Byte ch;

    for (i = 0; i < 6; ++i)
    {
        text(0, i + 8, hexstr((Word)(stk + mem_addr)));

        tmp[0] = ' ';
        tmp[1] = '\0';

        for (j = 0; j < 8; ++j)
        {
            ch = stat.memory[stk + j];
            strcat(tmp, hexstr(ch));
            strcat(tmp, " ");
        } // for

        for (j = 0; j < 8; ++j)
        {
            ch = stat.memory[stk + j];
            strcat(tmp, ascchr(ch));
        } // for

        text(5, i + 8, tmp);
        stk += 8;
    } // for

    redraw_cpuview_impl(stat);
}

void AbstractGui::redraw_cpuview_impl(const Mc6809CpuStatus &)
{
}

void AbstractGui::text(int x, int y, const char *str, int)
{
    strncpy(&cpustring[cpu_line_size * y + x], str, strlen(str));
}

void AbstractGui::clear_cpuview()
{
    int i;
    size_t size;

    for (i = 0; i <= CPU_LINES * cpu_line_size - 1; i++)
    {
        cpustring[i] = ' ';
    }

    cpustring[i] = '\0';
    size = strlen(cpu_line_delim);

    for (i = 1; i <= CPU_LINES; i++)
    {
        strncpy(&cpustring[i * cpu_line_size - size], cpu_line_delim, size);
    }
}

void AbstractGui::set_line_delim(const char *delim)
{
    size_t size = strlen(delim);

    if (size <= 2)
    {
        cpu_line_delim = delim;
        cpu_line_size = static_cast<int>(CPU_LINE_SIZE + size - 1);
    }
}

void AbstractGui::set_exit(bool b /* = true */)
{
    exit_flag = b;
}

void AbstractGui::update()
{
    static int display_block = 0;

    update_block(display_block);
    display_block += 1;
    display_block %= YBLOCKS;
}

void AbstractGui::set_new_state(Byte user_input)
{
    if (user_input != S_NO_CHANGE)
    {
        scheduler.set_new_state(user_input);
    }
}

void AbstractGui::set_bell(int)
{
}

void AbstractGui::output_to_terminal()
{
    if (terminalIO.is_terminal_supported() && switch_sp)
    {
        cpu.set_serpar(0xff);
    }
}

void AbstractGui::output_to_graphic()
{
    if (switch_sp)
    {
        cpu.set_serpar(0);
    }
}

GuiType AbstractGui::gui_type()
{
    return GuiType::NONE;
}

Word AbstractGui::get_divided_block() const
{
    if ((vico2.get_value() % BLOCKHEIGHT) == 0)
    {
        // With the current scrolling status there is no divided block
        return -1;
    }
    else
    {
        return (Word)vico2.get_value() / BLOCKHEIGHT;
    }
}

AbstractGui::AbstractGui(
    Mc6809 &x_cpu,
    Memory &x_memory,
    Scheduler &x_scheduler,
    Inout &x_inout,
    VideoControl1 &x_vico1,
    VideoControl2 &x_vico2,
    JoystickIO &x_joystickIO,
    KeyboardIO &x_keyboardIO,
    TerminalIO &x_terminalIO,
    struct sGuiOptions &x_options)
        : cpu(x_cpu)
        , memory(x_memory)
        , scheduler(x_scheduler)
        , inout(x_inout)
        , vico1(x_vico1)
        , vico2(x_vico2)
        , joystickIO(x_joystickIO)
        , keyboardIO(x_keyboardIO)
        , terminalIO(x_terminalIO)
        , options(x_options)
        , exit_flag(false)
        , cpu_line_size(CPU_LINE_SIZE)
        , cpu_line_delim("\n")
{
}

AbstractGui::~AbstractGui()
{
}

