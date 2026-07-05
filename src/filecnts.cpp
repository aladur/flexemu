/*
    filecnts.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2026  W. Schwotzer

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

#include "typedefs.h"
#include "misc1.h"
#include "filecntb.h"
#include "filecnts.h"
#include <cstdint>
#include <cstddef>
#include <cctype>
#include <algorithm>
#include <string>
#include <string_view>
#include <ostream>
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"

std::ostream& operator<<(std::ostream& os, const st_t &st)
{
    os << fmt::format("{:02X}-{:02X}", static_cast<Word>(st.trk),
            static_cast<Word>(st.sec));

    return os;
}

// Return the sector count on track 0 which has always single density (SD)
// except for a harddisk.
Word getTrack0SectorCount(DiskType type, int tracks, int sectors)
{
    if (type == DiskType::IMA && tracks == 255)
    {
        switch (sectors)
        {
            case 26: return 15; // 8 inch, single sided, double density.
            case 52: return 30; // 8 inch, single sided, double density.
            case 18: return 10; // 5 1/4 inch, single sided, double density.
            case 36: return 20; // 5 1/4 inch, double sided, double density.
            default:
                return static_cast<Word>(sectors); // single density,
                                                   // always same sector count.
        }
    }

    if (tracks >= 254)
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
            return static_cast<Word>(std::min(sectors, 15));
        }
        // This is a 8-inch double sided (DS) disk.
        return static_cast<Word>(std::min(sectors, 30));
    }

    // Assuming 5 1/4-inch or 3 1/2-inch disk. 34, 35, 40 or 80 tracks.
    if (sectors <= 18)
    {
        // This is a single sided (SS) disk.
        // This rule can be applied when creating a DSK container.
        // When reading a DSK container both 10 or 20 sectors
        // has to be supported.
        return static_cast<Word>(std::min(sectors, 10));
    }
    // This is a double sided (DS) disk.
    return static_cast<Word>(std::min(sectors, 20));
}

// Return the number of sides.
Word getSides(int tracks, int sectors)
{
    if (tracks >= 254)
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

Word getBytesPerSector(uint32_t sizecode)
{
    return 128U << (sizecode & 0x03U);
}

size_t getFileSize(const s_flex_header &header)
{
    return sizeof(s_flex_header) +
               ((header.sectors0 * header.sides0) +
                ((header.tracks - 1) * (header.sectors * header.sides))) *
                 getBytesPerSector(header.sizecode);
}

std::string getDiskName(const std::string &filename)
{
    if (!filename.empty() && filename[0] == '.')
    {
        return "FLEXDISK";
    }

    std::string diskname;
    std::size_t pos = 0U;

    diskname.reserve(FLEX_DISKNAME_LENGTH);
    while (true)
    {
        const auto ch_str = flx::nextUtf8Char(filename, pos);

        if (diskname.size() >= FLEX_DISKNAME_LENGTH || ch_str.empty() ||
            (ch_str.size() == 1U && ch_str[0] == '.'))
        {
            break;
        }

        if (ch_str.size() == 1 && (ch_str[0] >= ' ' && ch_str[0] <= '~'))
        {
            diskname.push_back(static_cast<char>(std::toupper(ch_str[0])));
            continue;
        }

        diskname.push_back('_');
    }

    return diskname;
}

