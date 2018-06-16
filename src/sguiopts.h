/*
sguiopts.h


flexemu, an MC6809 emulator running FLEX
Copyright (C) 2018  W. Schwotzer

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



#ifndef __sguiopts_h__
#define __sguiopts_h__

#include "misc1.h"
#include "flexemu.h"
#include <string>

// Maximum x-size of one emulated pixel on screen
#define MAX_PIXELSIZEX    (3)
// Maximum y-size of one emulated pixel on screen 
#define MAX_PIXELSIZEY    (4)

struct sGuiOptions
{
    int argc;
    char *const *argv;
    std::string color;
    std::string html_viewer;
    std::string doc_dir;
    int nColors; // Number of colors or gray scale values { 2, 8, 64 }
    bool isInverse; // Display inverse colors or gray scale values
    bool isSynchronized; // Use X11 in synchronized mode
    int switch_sp;
    int pixelSizeX; // x-size of one pixel on the screen
    int pixelSizeY; // y-size of one pixel on the screen
    GuiType guiType;
};

#endif
