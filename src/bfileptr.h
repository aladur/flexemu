/*
    bfileptr.h

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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

#ifndef BFILEPTR_INCLUDED
#define BFILEPTR_INCLUDED

#include <stdio.h>
#include <string>

class BFilePtr
{
public:
    BFilePtr();
    BFilePtr(const char *path, const char *mode = nullptr);
    BFilePtr(const BFilePtr &) = delete;
    BFilePtr(BFilePtr &&) noexcept;
    ~BFilePtr();
    BFilePtr &operator= (const BFilePtr &) = delete;
    BFilePtr &operator= (BFilePtr &&) noexcept;

    bool Close();
    const char *GetPath() const
    {
        return path.c_str();
    }

    const char *GetMode() const
    {
        return mode.c_str();
    }

    operator FILE *() const
    {
        return fp;
    }

private:
    std::string path;
    std::string mode;
    FILE *fp;
};
#endif

