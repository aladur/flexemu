/*
    bdir.h


    Basic class used for directory functions
    Copyright (C) 1999-2025  W. Schwotzer

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

#ifndef BDIR_INCLUDED
#define BDIR_INCLUDED

#ifdef _WIN32
    #pragma warning (disable: 4786)
#endif
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;


using PathList_t = std::vector<std::string>;

class BDirectory
{
private:
    fs::path path;

public:
    static PathList_t GetSubDirectories(const fs::path &p_path);
    static PathList_t GetFiles(const fs::path &p_path);

    BDirectory() = default;
    explicit BDirectory(fs::path &p_path) : path(p_path) { };

    inline void SetPath(fs::path &p_path)
    {
        path = p_path;
    };
    inline const fs::path &GetPath() const
    {
        return path;
    };
    PathList_t GetSubDirectories() const;
    PathList_t GetFiles() const;
};

#endif // BDIR_INCLUDED
