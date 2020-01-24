/*
    filecnts.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020  W. Schwotzer

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

#include "filecnts.h"
#include <ostream>
#include <iomanip>
#include <string>
#include <sstream>

std::ostream& operator<<(std::ostream& os, const  st_t &st)
{
    auto previous_flags = os.flags();
    auto previous_fill = os.fill('0');

    os <<
        std::hex << std::uppercase <<
        std::setw(2) << (Word)st.trk << "-" <<
        std::setw(2) << (Word)st.sec;
    os.fill(previous_fill);
    os.flags(previous_flags);

    return os;
}

// Return the sector count on track 0 which has always single density (SD)
// except for a harddisk.
int getTrack0SectorCount(int tracks, int sectors)
{
    if (tracks == 256)
    {
        // This is a harddisk. Assuming same density for all tracks.
        return sectors;
    }

    // Check for 8-inch disk.
    if (tracks == 77)
    {
        if (sectors <= 26)
        {
            // This is a 8-inch single sided (SS) disk.
            return 15;
        }
        // This is a 8-inch double sided (DS) disk.
        return 30;
    }

    // Assuming 5 1/4-inch or 3 1/2-inch disk. 34, 35, 40 or 80 tracks.
    if (sectors <= 18)
    {
        // This is a single sided (SS) disk.
        // This rule can be applied when creating a DSK container.
        // When reading a DSK container both 10 or 20 sectors
        // has to be supported.
        return 10;
    }
    // This is a double sided (DS) disk.
    return 20;
}

