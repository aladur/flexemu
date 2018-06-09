/*
    bfileptr.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

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

BFilePtr::BFilePtr() :
    pPath(nullptr), pMode(nullptr), fp(nullptr), responsible(true)
{
}

BFilePtr::BFilePtr(const char *path, const char *mode) :
    pPath(nullptr), pMode(nullptr), fp(nullptr), responsible(true)
{
    struct stat  sbuf;

    if (path != nullptr)
    {
        pPath = new char[strlen(path) + 1];
        strcpy(pPath, path);
    }

    if (mode != nullptr)
    {
        pMode = new char[strlen(mode) + 1];
        strcpy(pMode, mode);
    }

    if (!stat(path, &sbuf) && !S_ISREG(sbuf.st_mode))
    {
        return;
    }

    fp = fopen(path, mode);
}

BFilePtr::BFilePtr(const BFilePtr &src) :
    pPath(nullptr), pMode(nullptr), fp(nullptr), responsible(true)
{
    fp = src.fp;

    if (src.pPath != nullptr)
    {
        pPath = new char[strlen(src.pPath) + 1];
        strcpy(pPath, src.pPath);
    }

    if (src.pMode != nullptr)
    {
        pMode = new char[strlen(src.pMode) + 1];
        strcpy(pMode, src.pMode);
    }

    // only take over responsibility for closing file
    // if source filepointer had it
    responsible     = src.responsible;
    src.responsible = false;
}

BFilePtr &BFilePtr::operator=(const BFilePtr &src)
{
    if (&src != this)
    {
        Close();

        fp = src.fp;

        if (src.pPath != nullptr)
        {
            pPath = new char[strlen(src.pPath) + 1];
            strcpy(pPath, src.pPath);
        }

        if (src.pMode != nullptr)
        {
            pMode = new char[strlen(src.pMode) + 1];
            strcpy(pMode, src.pMode);
        }

        // only take over responsibility for closing file
        // if source filepointer had it
        responsible     = src.responsible;
        src.responsible = false;
    }

    return *this;
}

BFilePtr::~BFilePtr()
{
    Close();
}

int BFilePtr::Close()
{
    int result = 0;

    if (responsible && fp != nullptr)
    {
        result = fclose(fp);
    }

    // make file pointer invalid independant
    // of the responsibility
    fp = nullptr;

    delete [] pPath;
    pPath = nullptr;
    delete [] pMode;
    pMode = nullptr;

    return result;
}

