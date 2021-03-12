/*
    soptions.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2021  W. Schwotzer

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



#ifndef SOPTIONS_INCLUDED
#define SOPTIONS_INCLUDED

#include <string>
#include <array>

// Maximum size of one emulated pixel on screen
#define MAX_PIXELSIZE (5)

struct sOptions
{
    sOptions() = default;

    std::string drive[4];
    std::array<std::string, 2> mdcrDrives;
    std::string hex_file;
    std::string disk_dir;
    std::string startup_command;
    bool isRamExtension;  // Use RAM extension cards/No RAM extension
    bool isHiMem;         // Use 2 x 288K RAM extension/2 x 96 K RAM ext.
    bool isFlexibleMmu;   // Use flexible MMU/Normal MMU
    bool isEurocom2V5;    // Emulate an Eurocom II/V5 (instead of Eurocom II/V7)
    bool use_undocumented;
    bool useRtc;
    bool term_mode;
    short int reset_key; // must be short int because of sscanf !!!
    float frequency;

    // User interface options
    std::string color; // color name or "default" for multi color palette.
    std::string doc_dir; // Directory containing html documenation.
    int nColors; // Number of colors or gray scale values { 2, 8, 64 }.
    bool isInverse; // Display inverse colors or gray scale values.
    int pixelSize; // Size of one pixel on the screen { 1, 2, 3, 4, 5 }.
                   // It depends on the screen dimensions on which flexemu
                   // is executed.
};

#endif

