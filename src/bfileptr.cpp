/*
    bfileptr.cpp

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
#include "misc1.h"
#include <sys/stat.h>
#include "bfileptr.h"

// file pointer object usually initialize in the
// member initialization of a class
// The developper does not have to be concerned about
// closing the file

BFilePtr::BFilePtr() : fp(nullptr)
{
}

BFilePtr::BFilePtr(const std::string &p_path, const std::string &p_mode)
    : mode(p_mode)
    , fp(nullptr)
{
    struct stat sbuf{};

    if (isAbsolutePath(p_path))
    {
        path = p_path;
    }
    else
    {
        path = getCurrentPath();
        if (!path.empty() && !endsWithPathSeparator(path))
        {
            path += PATHSEPARATORSTRING;
        }
        path += p_path;
    }

    if (!stat(path.c_str(), &sbuf) && !S_ISREG(sbuf.st_mode))
    {
        return;
    }

    fp = fopen(path.c_str(), mode.c_str());
}

BFilePtr::BFilePtr(BFilePtr &&src) noexcept
    : path(std::move(src.path))
    , mode(std::move(src.mode))
    , fp(src.fp)
{
    src.fp = nullptr;
}

BFilePtr &BFilePtr::operator=(BFilePtr &&src) noexcept
{
    if (&src != this)
    {
        Close();

        fp = src.fp;
        path = src.path;
        mode = src.mode;

        src.fp = nullptr;
        src.path.clear();
        src.mode.clear();
    }

    return *this;
}

BFilePtr::~BFilePtr()
{
    Close();
}

bool BFilePtr::Close()
{
    bool result = true;

    if (fp != nullptr)
    {
        result = (fclose(fp) == 0);
    }

    fp = nullptr;
    path.clear();
    mode.clear();

    return result;
}

