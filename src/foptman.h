/*
    foptman.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

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

#ifndef FOPTMAN_INCLUDED
#define FOPTMAN_INCLUDED

#include "misc1.h"
#include "soptions.h"
#include <iostream>


static constexpr std::array<FlexemuOptionId, 4> canFormatDriveOptionId
{{
    FlexemuOptionId::CanFormatDrive0,
    FlexemuOptionId::CanFormatDrive1,
    FlexemuOptionId::CanFormatDrive2,
    FlexemuOptionId::CanFormatDrive3,
}};

class FlexemuOptions
{
public:
    static void PrintHelp(std::ostream &os);
    static void InitOptions(struct sOptions &options);
    static void GetOptions(struct sOptions &options);
    static void GetCommandlineOptions(
        struct sOptions &options,
        int argc,
        /* Parameter comes from main(). */
        /* NOLINTNEXTLINE(modernize-avoid-c-arrays) */
        char *const argv[]);
    static void WriteOptions(
        const struct sOptions &options,
        bool ifNotExists = false,
        bool readWriteOptionsOnly = false);

private:
#ifdef _WIN32
    static void WriteOptionsToRegistry(
        const struct sOptions &options,
        const std::vector<FlexemuOptionId> &optionIds,
        bool ifNotExists = false);
#endif
#ifdef UNIX
    static void WriteOptionsToFile(
        const struct sOptions &options,
        const std::vector<FlexemuOptionId> &optionIds,
        const std::string &fileName,
        bool ifNotExists = false);
#endif
};

#endif // FOPTMAN_INCLUDED

