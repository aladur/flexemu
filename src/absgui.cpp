/*
    absgui.cc: abstract graphical user interface


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2004  W. Schwotzer

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
#include "e2video.h"
#include "mc6809.h"
#include "memory.h"
#include "inout.h"
#include "schedule.h"
#include "mc6809st.h"

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

void AbstractGui::initialize(struct sGuiOptions *pOptions)
{
    switch_sp = 0;
    program_name = "";
    timebase = 20; // timer event every x milliseconds

    if (pOptions != NULL)
    {
        program_name    = pOptions->argv[0];
        switch_sp   = pOptions->switch_sp;
        pixelSizeX    = pOptions->pixelSizeX;
        pixelSizeY    = pOptions->pixelSizeY;
        nColors     = pOptions->nColors;
        withColorScale  = !stricmp(pOptions->color.c_str(), "default");
        color       = pOptions->color;
    }

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
        pen[i] = i;
    }
} // initialize_conv_tables

void AbstractGui::main_loop(void)
{
    // to be implemented by subclass
}

void AbstractGui::update_cpuview(const Mc6809CpuStatus &stat)
{
    redraw_cpuview(stat);
}

void AbstractGui::CopyToZPixmap(int,
                                void *dest, const Byte *src,
                                int depth, const unsigned long *pen)
{
    register int j;         /* plane counter                 */
    register Word srcPixel; /* calculated pixel in video ram */
    register int i;         /* index into video ram          */
    register Byte b1 = 0, b3 = 0, b5 = 0; /* byte buffers for each plane */
    const Byte *vram;       /* pointer into video ram        */
    Byte     *dest8;        /* destination pointer in image  */
    Word     *dest16;       /* destination pointer in image  */
    DWord    *dest32;
    unsigned long q;

    vram = src;
    dest8  = (Byte *)dest;
    dest16 = (Word *)dest;
    dest32 = (DWord *)dest;

    for (i = 0; i < YBLOCK_SIZE; i++)
    {
        register Byte b0, b2, b4;       /* byte buffers for each plane */

        b0 = vram[0];
        b2 = vram[VIDEORAM_SIZE];
        b4 = vram[VIDEORAM_SIZE * 2];

        if (nColors > 8)
        {
            b1 = vram[VIDEORAM_SIZE * 3];
            b3 = vram[VIDEORAM_SIZE * 4];
            b5 = vram[VIDEORAM_SIZE * 5];
        }

        vram++;

        /* Use MSBit first */
        for (j = 128; j; j >>= 1)
        {
            srcPixel = 0;

            if (nColors > 8)
            {
                if (b0 & j)
                {
                    srcPixel |= 32;    // 0x0C, green high
                }

                if (b2 & j)
                {
                    srcPixel |= 16;    // 0x0D, red high
                }

                if (b4 & j)
                {
                    srcPixel |= 8;    // 0x0E, blue high
                }

                if (b1 & j)
                {
                    srcPixel |= 4;    // 0x04, green low
                }

                if (b3 & j)
                {
                    srcPixel |= 2;    // 0x05, red low
                }

                if (b5 & j)
                {
                    srcPixel |= 1;    // 0x06, blue low
                }
            }
            else
            {
                if (b0 & j)
                {
                    srcPixel |= 32;    // 0x0C, green high
                }

                if (b2 & j)
                {
                    srcPixel |= 8;    // 0x0D, red high
                }

                if (b4 & j)
                {
                    srcPixel |= 2;    // 0x0E, blue high
                }
            }

            q = pen[srcPixel];

            if (depth == 16)
                switch (pixelSizeX << 4 | pixelSizeY)
                {
                    case 0x11:
                        *(dest16++) = (Word)q;
                        break;

                    case 0x12:
                        *(dest16) = (Word)q;
                        *(dest16 + WINDOWWIDTH) = (Word)q;
                        dest16++;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest16 += WINDOWWIDTH;
                        }

                        break;

                    case 0x13:
                        *(dest16) = (Word)q;
                        *(dest16 + WINDOWWIDTH) = (Word)q;
                        *(dest16 + 2 * WINDOWWIDTH) = (Word)q;
                        dest16++;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest16 += 2 * WINDOWWIDTH;
                        }

                        break;

                    case 0x14:
                        *(dest16) = (Word)q;
                        *(dest16 + WINDOWWIDTH) = (Word)q;
                        *(dest16 + 2 * WINDOWWIDTH) = (Word)q;
                        *(dest16 + 3 * WINDOWWIDTH) = (Word)q;
                        dest16++;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest16 += 3 * WINDOWWIDTH;
                        }

                        break;

                    case 0x21:
                        *(dest16++) = (Word)q;
                        *(dest16++) = (Word)q;
                        break;

                    case 0x22:
                        *(dest16) = (Word)q;
                        *(dest16 + 1) = (Word)q;
                        *(dest16 + 2 * WINDOWWIDTH) = (Word)q;
                        *(dest16 + 2 * WINDOWWIDTH + 1) = (Word)q;
                        dest16 += 2;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest16 += 2 * WINDOWWIDTH;
                        }

                        break;

                    case 0x23:
                        *(dest16) = (Word)q;
                        *(dest16 + 1) = (Word)q;
                        *(dest16 + 2 * WINDOWWIDTH) = (Word)q;
                        *(dest16 + 2 * WINDOWWIDTH + 1) = (Word)q;
                        *(dest16 + 4 * WINDOWWIDTH) = (Word)q;
                        *(dest16 + 4 * WINDOWWIDTH + 1) = (Word)q;
                        dest16 += 2;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest16 += 4 * WINDOWWIDTH;
                        }

                        break;

                    case 0x24:
                        *(dest16) = (Word)q;
                        *(dest16 + 1) = (Word)q;
                        *(dest16 + 2 * WINDOWWIDTH) = (Word)q;
                        *(dest16 + 2 * WINDOWWIDTH + 1) = (Word)q;
                        *(dest16 + 4 * WINDOWWIDTH) = (Word)q;
                        *(dest16 + 4 * WINDOWWIDTH + 1) = (Word)q;
                        *(dest16 + 6 * WINDOWWIDTH) = (Word)q;
                        *(dest16 + 6 * WINDOWWIDTH + 1) = (Word)q;
                        dest16 += 2;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest16 += 6 * WINDOWWIDTH;
                        }

                        break;

                    case 0x31:
                        *(dest16++) = (Word)q;
                        *(dest16++) = (Word)q;
                        *(dest16++) = (Word)q;
                        break;

                    case 0x32:
                        *(dest16) = (Word)q;
                        *(dest16 + 1) = (Word)q;
                        *(dest16 + 2) = (Word)q;
                        *(dest16 + 3 * WINDOWWIDTH) = (Word)q;
                        *(dest16 + 3 * WINDOWWIDTH + 1) = (Word)q;
                        *(dest16 + 3 * WINDOWWIDTH + 2) = (Word)q;
                        dest16 += 3;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest16 += 3 * WINDOWWIDTH;
                        }

                        break;

                    case 0x33:
                        *(dest16) = (Word)q;
                        *(dest16 + 1) = (Word)q;
                        *(dest16 + 2) = (Word)q;
                        *(dest16 + 3 * WINDOWWIDTH) = (Word)q;
                        *(dest16 + 3 * WINDOWWIDTH + 1) = (Word)q;
                        *(dest16 + 3 * WINDOWWIDTH + 2) = (Word)q;
                        *(dest16 + 6 * WINDOWWIDTH) = (Word)q;
                        *(dest16 + 6 * WINDOWWIDTH + 1) = (Word)q;
                        *(dest16 + 6 * WINDOWWIDTH + 2) = (Word)q;
                        dest16 += 3;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest16 += 6 * WINDOWWIDTH;
                        }

                        break;

                    case 0x34:
                        *(dest16) = (Word)q;
                        *(dest16 + 1) = (Word)q;
                        *(dest16 + 2) = (Word)q;
                        *(dest16 + 3 * WINDOWWIDTH) = (Word)q;
                        *(dest16 + 3 * WINDOWWIDTH + 1) = (Word)q;
                        *(dest16 + 3 * WINDOWWIDTH + 2) = (Word)q;
                        *(dest16 + 6 * WINDOWWIDTH) = (Word)q;
                        *(dest16 + 6 * WINDOWWIDTH + 1) = (Word)q;
                        *(dest16 + 6 * WINDOWWIDTH + 2) = (Word)q;
                        *(dest16 + 9 * WINDOWWIDTH) = (Word)q;
                        *(dest16 + 9 * WINDOWWIDTH + 1) = (Word)q;
                        *(dest16 + 9 * WINDOWWIDTH + 2) = (Word)q;
                        dest16 += 3;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest16 += 9 * WINDOWWIDTH;
                        }

                        break;
                }
            else if (depth == 8)
                switch (pixelSizeX << 4 | pixelSizeY)
                {
                    case 0x11: // single width, single height
                        *(dest8++) = (Byte)q;
                        break;

                    case 0x12: // single width, double height
                        *(dest8) = (Byte)q;
                        *(dest8 + WINDOWWIDTH) = (Byte)q;
                        dest8++;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest8 += WINDOWWIDTH;
                        }

                        break;

                    case 0x13: // single width, tripple height
                        *(dest8) = (Byte)q;
                        *(dest8 + WINDOWWIDTH) = (Byte)q;
                        *(dest8 + 2 * WINDOWWIDTH) = (Byte)q;
                        dest8++;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest8 += 2 * WINDOWWIDTH;
                        }

                        break;

                    case 0x14: // single width, quadrupple height
                        *(dest8) = (Byte)q;
                        *(dest8 + WINDOWWIDTH) = (Byte)q;
                        *(dest8 + 2 * WINDOWWIDTH) = (Byte)q;
                        *(dest8 + 3 * WINDOWWIDTH) = (Byte)q;
                        dest8++;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest8 += 3 * WINDOWWIDTH;
                        }

                        break;

                    case 0x21: // double width, single heigth
                        *(dest8++) = (Byte)q;
                        *(dest8++) = (Byte)q;
                        break;

                    case 0x22: // double width, double height
                        *(dest8) = (Byte)q;
                        *(dest8 + 1) = (Byte)q;
                        *(dest8 + 2 * WINDOWWIDTH) = (Byte)q;
                        *(dest8 + 2 * WINDOWWIDTH + 1) = (Byte)q;
                        dest8 += 2;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest8 += 2 * WINDOWWIDTH;
                        }

                        break;

                    case 0x23: // double width, tripple height
                        *(dest8) = (Byte)q;
                        *(dest8 + 1) = (Byte)q;
                        *(dest8 + 2 * WINDOWWIDTH) = (Byte)q;
                        *(dest8 + 2 * WINDOWWIDTH + 1) = (Byte)q;
                        *(dest8 + 4 * WINDOWWIDTH) = (Byte)q;
                        *(dest8 + 4 * WINDOWWIDTH + 1) = (Byte)q;
                        dest8 += 2;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest8 += 4 * WINDOWWIDTH;
                        }

                        break;

                    case 0x24: // double width, quadrupple height
                        *(dest8) = (Byte)q;
                        *(dest8 + 1) = (Byte)q;
                        *(dest8 + 2 * WINDOWWIDTH) = (Byte)q;
                        *(dest8 + 2 * WINDOWWIDTH + 1) = (Byte)q;
                        *(dest8 + 4 * WINDOWWIDTH) = (Byte)q;
                        *(dest8 + 4 * WINDOWWIDTH + 1) = (Byte)q;
                        *(dest8 + 6 * WINDOWWIDTH) = (Byte)q;
                        *(dest8 + 6 * WINDOWWIDTH + 1) = (Byte)q;
                        dest8 += 2;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest8 += 6 * WINDOWWIDTH;
                        }

                        break;

                    case 0x31: // triple width, single height
                        *(dest8) = (Byte)q;
                        *(dest8 + 1) = (Byte)q;
                        *(dest8 + 2) = (Byte)q;
                        dest8 += 3;
                        break;

                    case 0x32: // triple width, double height
                        *(dest8) = (Byte)q;
                        *(dest8 + 1) = (Byte)q;
                        *(dest8 + 2) = (Byte)q;
                        *(dest8 + 3 * WINDOWWIDTH) = (Byte)q;
                        *(dest8 + 3 * WINDOWWIDTH + 1) = (Byte)q;
                        *(dest8 + 3 * WINDOWWIDTH + 2) = (Byte)q;
                        dest8 += 3;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest8 += 3 * WINDOWWIDTH;
                        }

                        break;

                    case 0x33: // triple width, triple height
                        *(dest8) = (Byte)q;
                        *(dest8 + 1) = (Byte)q;
                        *(dest8 + 2) = (Byte)q;
                        *(dest8 + 3 * WINDOWWIDTH) = (Byte)q;
                        *(dest8 + 3 * WINDOWWIDTH + 1) = (Byte)q;
                        *(dest8 + 3 * WINDOWWIDTH + 2) = (Byte)q;
                        *(dest8 + 6 * WINDOWWIDTH) = (Byte)q;
                        *(dest8 + 6 * WINDOWWIDTH + 1) = (Byte)q;
                        *(dest8 + 6 * WINDOWWIDTH + 2) = (Byte)q;
                        dest8 += 3;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest8 += 6 * WINDOWWIDTH;
                        }

                        break;

                    case 0x34: // triple width, quadruple height
                        *(dest8) = (Byte)q;
                        *(dest8 + 1) = (Byte)q;
                        *(dest8 + 2) = (Byte)q;
                        *(dest8 + 3 * WINDOWWIDTH) = (Byte)q;
                        *(dest8 + 3 * WINDOWWIDTH + 1) = (Byte)q;
                        *(dest8 + 3 * WINDOWWIDTH + 2) = (Byte)q;
                        *(dest8 + 6 * WINDOWWIDTH) = (Byte)q;
                        *(dest8 + 6 * WINDOWWIDTH + 1) = (Byte)q;
                        *(dest8 + 6 * WINDOWWIDTH + 2) = (Byte)q;
                        *(dest8 + 9 * WINDOWWIDTH) = (Byte)q;
                        *(dest8 + 9 * WINDOWWIDTH + 1) = (Byte)q;
                        *(dest8 + 9 * WINDOWWIDTH + 2) = (Byte)q;
                        dest8 += 3;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest8 += 9 * WINDOWWIDTH;
                        }

                        break;
                }
            // assuming depth of 24 or 32
            else
            {
                switch (pixelSizeX << 4 | pixelSizeY)
                {
                    case 0x11: // single width, single height
                        *(dest32++) = q;
                        break;

                    case 0x12: // single width, double height
                        *(dest32) = q;
                        *(dest32 + WINDOWWIDTH) = q;
                        dest32++;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest32 += WINDOWWIDTH;
                        }

                        break;

                    case 0x13: // single width, tripple height
                        *(dest32) = q;
                        *(dest32 + WINDOWWIDTH) = q;
                        *(dest32 + 2 * WINDOWWIDTH) = q;
                        dest32++;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest32 += 2 * WINDOWWIDTH;
                        }

                        break;

                    case 0x14: // single width, quadrupple height
                        *(dest32) = q;
                        *(dest32 + WINDOWWIDTH) = q;
                        *(dest32 + 2 * WINDOWWIDTH) = q;
                        *(dest32 + 3 * WINDOWWIDTH) = q;
                        dest32++;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest32 += 3 * WINDOWWIDTH;
                        }

                        break;

                    case 0x21: // double width, single heigth
                        *(dest32++) = q;
                        *(dest32++) = q;
                        break;

                    case 0x22: // double width, double height
                        *(dest32) = q;
                        *(dest32 + 1) = q;
                        *(dest32 + 2 * WINDOWWIDTH) = q;
                        *(dest32 + 2 * WINDOWWIDTH + 1) = q;
                        dest32 += 2;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest32 += 2 * WINDOWWIDTH;
                        }

                        break;

                    case 0x23: // double width, tripple height
                        *(dest32) = q;
                        *(dest32 + 1) = q;
                        *(dest32 + 2 * WINDOWWIDTH) = q;
                        *(dest32 + 2 * WINDOWWIDTH + 1) = q;
                        *(dest32 + 4 * WINDOWWIDTH) = q;
                        *(dest32 + 4 * WINDOWWIDTH + 1) = q;
                        dest32 += 2;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest32 += 4 * WINDOWWIDTH;
                        }

                        break;

                    case 0x24: // double width, quadrupple height
                        *(dest32) = q;
                        *(dest32 + 1) = q;
                        *(dest32 + 2 * WINDOWWIDTH) = q;
                        *(dest32 + 2 * WINDOWWIDTH + 1) = q;
                        *(dest32 + 4 * WINDOWWIDTH) = q;
                        *(dest32 + 4 * WINDOWWIDTH + 1) = q;
                        *(dest32 + 6 * WINDOWWIDTH) = q;
                        *(dest32 + 6 * WINDOWWIDTH + 1) = q;
                        dest32 += 2;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest32 += 6 * WINDOWWIDTH;
                        }

                        break;

                    case 0x31: // triple width, single height
                        *(dest32++) = q;
                        *(dest32++) = q;
                        *(dest32++) = q;
                        break;

                    case 0x32: // triple width, double height
                        *(dest32) = q;
                        *(dest32 + 1) = q;
                        *(dest32 + 2) = q;
                        *(dest32 + 3 * WINDOWWIDTH) = q;
                        *(dest32 + 3 * WINDOWWIDTH + 1) = q;
                        *(dest32 + 3 * WINDOWWIDTH + 2) = q;
                        dest32 += 3;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest32 += 3 * WINDOWWIDTH;
                        }

                        break;

                    case 0x33: // triple width, triple height
                        *(dest32) = q;
                        *(dest32 + 1) = q;
                        *(dest32 + 2) = q;
                        *(dest32 + 3 * WINDOWWIDTH) = q;
                        *(dest32 + 3 * WINDOWWIDTH + 1) = q;
                        *(dest32 + 3 * WINDOWWIDTH + 2) = q;
                        *(dest32 + 6 * WINDOWWIDTH) = q;
                        *(dest32 + 6 * WINDOWWIDTH + 1) = q;
                        *(dest32 + 6 * WINDOWWIDTH + 2) = q;
                        dest32 += 3;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest32 += 6 * WINDOWWIDTH;
                        }

                        break;

                    case 0x34: // triple width, quadruple height
                        *(dest32) = q;
                        *(dest32 + 1) = q;
                        *(dest32 + 2) = q;
                        *(dest32 + 3 * WINDOWWIDTH) = q;
                        *(dest32 + 3 * WINDOWWIDTH + 1) = q;
                        *(dest32 + 3 * WINDOWWIDTH + 2) = q;
                        *(dest32 + 6 * WINDOWWIDTH) = q;
                        *(dest32 + 6 * WINDOWWIDTH + 1) = q;
                        *(dest32 + 6 * WINDOWWIDTH + 2) = q;
                        *(dest32 + 9 * WINDOWWIDTH) = q;
                        *(dest32 + 9 * WINDOWWIDTH + 1) = q;
                        *(dest32 + 9 * WINDOWWIDTH + 2) = q;
                        dest32 += 3;

                        if ((i % RASTERLINE_SIZE) ==
                            (RASTERLINE_SIZE - 1) && j == 1)
                        {
                            dest32 += 9 * WINDOWWIDTH;
                        }

                        break;
                }
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
        if (!cpu->is_bp_set(i))
        {
            text(25, 5 + i, "    ", bp_input[i]);
        }
        else
            text(25, 5 + i,
                 hexstr((Word)cpu->get_bp(i)),
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

void AbstractGui::clear_cpuview(void)
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

void AbstractGui::update(void)
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
        schedy->set_new_state(user_input);
    }
}

void AbstractGui::set_bell(int)
{
}

void AbstractGui::output_to_terminal(void)
{
    if (io->is_terminal_supported() && switch_sp)
    {
        cpu->set_serpar(0xff);
    }
}

void AbstractGui::output_to_graphic(void)
{
    if (switch_sp)
    {
        cpu->set_serpar(0);
    }
}

int AbstractGui::gui_type(void)
{
    return -1;
}

AbstractGui::AbstractGui(
    Mc6809    *x_cpu,
    Memory    *x_memory,
    Scheduler *x_sched,
    Inout     *x_io,
    E2video   *x_video,
    struct sGuiOptions *x_pOptions) :
    cpu(x_cpu), memory(x_memory), schedy(x_sched), io(x_io),
    e2video(x_video), pOptions(x_pOptions), exit_flag(false),
    cpu_line_size(CPU_LINE_SIZE), cpu_line_delim("\n")
{
    if (e2video != NULL)
    {
        e2video->resetIo();
    }
}

AbstractGui::~AbstractGui(void)
{
}

