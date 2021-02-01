/*
    brcfile.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2021  W. Schwotzer

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

#ifndef BRCFILE_INCLUDED
#define BRCFILE_INCLUDED

#include <string>

enum
{
    BRC_NO_ERROR = 0,
    BRC_NOT_FOUND = 99,
    BRC_NO_INTEGER = 100
};

class BRcFile
{
public:
    BRcFile();
    BRcFile(const char *aFileName);
    ~BRcFile();

    void SetFileName(const char *aFileName);
    int SetValue(const char *key, const char *value);
    int SetValue(const char *key, int value);
    int GetValue(const char *key, std::string &value, int *isInteger = nullptr);
    int GetValue(const char *key, int &value);
    int Initialize();

private:
    std::string fileName;
};

#endif

