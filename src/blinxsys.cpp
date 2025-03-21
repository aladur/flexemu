/*
    blinxsys.cpp


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


#include "blinxsys.h"
#include "bdir.h"
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;


#ifdef __linux__
std::string BLinuxSysInfo::ToString(BLinuxSysInfoType type)
{
    switch (type)
    {
        case BLinuxSysInfoType::LED:
            return "LED";
    }

    return "unknown";
}

std::string BLinuxSysInfo::Read(BLinuxSysInfoType type,
                                const std::string &subtype) const
{
    fs::path path;
    std::vector<std::string> subdirs;
    bool isPathAvailable = true;
    const auto id = ToString(type).append(" ").append(subtype);

    if (pathCache.find(id) == pathCache.end())
    {
        isPathAvailable = false;
        switch (type)
        {
            case BLinuxSysInfoType::LED:
                path = fs::u8path(u8"/sys/class/leds");
                subdirs = BDirectory::GetSubDirectories(path);
                for (const std::string &subdir : subdirs)
                {
                    auto pos = subdir.find(subtype);
                    if (pos != std::string::npos)
                    {
                        if (pos == (subdir.size() - subtype.size()))
                        {
                            path = path / subdir / u8"brightness";
                            pathCache[id] = path;
                            isPathAvailable = true;
                            break;
                        }
                    }
                }
                break;
            default:
                return "";
        }
    }

    if (isPathAvailable)
    {
        path = pathCache.at(id);
        std::ifstream fs(path);
        if (fs.is_open())
        {
            std::string linebuffer;

            std::getline(fs, linebuffer);
            fs.close();

            return linebuffer;
        }
    }

    return "";
}
#endif

