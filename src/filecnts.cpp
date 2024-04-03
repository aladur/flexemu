/*
    filecnts.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2024  W. Schwotzer

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

std::ostream& operator<<(std::ostream& os, const st_t &st)
{
    auto previous_flags = os.flags();
    auto previous_fill = os.fill('0');

    os <<
        std::hex << std::uppercase <<
        std::setw(2) << static_cast<Word>(st.trk) << "-" <<
        std::setw(2) << static_cast<Word>(st.sec);
    os.fill(previous_fill);
    os.flags(previous_flags);

    return os;
}

// Return the sector count on track 0 which has always single density (SD)
// except for a harddisk.
Word getTrack0SectorCount(int tracks, int sectors)
{
    if (tracks >= 255)
    {
        // This is a harddisk. Assuming same density for all tracks.
        return static_cast<Word>(sectors);
    }

    // Check for 8-inch disk.
    if (tracks == 77)
    {
        if (sectors <= 26)
        {
            // This is a 8-inch single sided (SS) disk.
            return 15U;
        }
        // This is a 8-inch double sided (DS) disk.
        return 30U;
    }

    // Assuming 5 1/4-inch or 3 1/2-inch disk. 34, 35, 40 or 80 tracks.
    if (sectors <= 18)
    {
        // This is a single sided (SS) disk.
        // This rule can be applied when creating a DSK container.
        // When reading a DSK container both 10 or 20 sectors
        // has to be supported.
        return 10U;
    }
    // This is a double sided (DS) disk.
    return 20U;
}

// Return the number of sides.
Word getSides(int tracks, int sectors)
{
    if (tracks >= 255)
    {
        // There are no details available about how many sides a hard disk
        // used by FLEX has => Return the default.
        return 1U;
    }

    // Check for 8-inch disk.
    if (tracks == 77)
    {
        if (sectors <= 26)
        {
            // This is a 8-inch single sided (SS) disk.
            return 1U;
        }
        // This is a 8-inch double sided (DS) disk if sector count is a
        // multiple of 2.
        return !(sectors % 2) ? 2 : 1;
    }

    // Assuming 5 1/4-inch or 3 1/2-inch disk. 34, 35, 40 or 80 tracks.
    if (sectors <= 18)
    {
        // This is a single sided (SS) disk.
        // This rule can be applied when creating a FLX container.
        // When reading a FLX container both single or double sided
        // has to be supported.
        return 1U;
    }
    // This is a double sided (DS) disk if sector count is a
    // multiple of 2.
    return !(sectors % 2) ? 2U : 1U;
}

Word getBytesPerSector(int sizecode)
{
    return 128U << (sizecode & 0x03);
}

size_t getFileSize(const s_flex_header &header)
{
    return sizeof(s_flex_header) +
               ((header.sectors0 * header.sides0) +
                ((header.tracks - 1) * (header.sectors * header.sides))) *
                 getBytesPerSector(header.sizecode);
}

