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

#include "misc1.h"
#ifdef _WIN32
    #pragma warning (disable: 4786)
#endif
#include <vector>
#include <string>


using PathList_t = std::vector<std::string>;

class BDirectory
{
private:
    std::string path;

public:
    static bool Exists(const std::string &p_path);
    static bool Remove(const std::string &p_path);
    static bool Create(const std::string &p_path, int mode = 0755);
    static bool RemoveRecursive(const std::string &p_path);
    static PathList_t GetSubDirectories(const std::string &p_path);
    static PathList_t GetFiles(const std::string &p_path);

    BDirectory() = default;
    explicit BDirectory(std::string &p_path) : path(p_path) { };
    ~BDirectory() = default;

    inline void SetPath(std::string &p_path)
    {
        path = p_path;
    };
    inline const std::string &GetPath() const
    {
        return path;
    };
    bool Exists() const;
    bool Remove() const;
    bool Create(int mode = 0755) const;
    bool RemoveRecursive() const;
    PathList_t GetSubDirectories() const;
    PathList_t GetFiles() const;
};

#endif // BDIR_INCLUDED
