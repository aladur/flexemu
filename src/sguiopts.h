/*
    sguiopts.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2021  W. Schwotzer

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



#ifndef SGUIOPTS_INCLUDED
#define SGUIOPTS_INCLUDED

#include "misc1.h"
#include "flexemu.h"
#include <string>

// Maximum size of one emulated pixel on screen
#define MAX_PIXELSIZE     (5)

// The values for argc, argv and doc_dir are set once
// and should not be edited.
struct sGuiOptions
{
    int argc;
    char *const *argv;
    std::string color;
    std::string doc_dir; // Directory containing html documenation
    int nColors; // Number of colors or gray scale values { 2, 8, 64 }
    bool isInverse; // Display inverse colors or gray scale values
    int pixelSize; // Size of one pixel on the screen { 1, 2, 3, 4, 5 }.
                   // It depends on the screen dimensions on which flexemu
                   // is executed.
};

#endif
