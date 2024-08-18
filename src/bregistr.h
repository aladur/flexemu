/*
    bregistr.h

    Basic class containing a windows registry handle
    Copyright (C) 1999-2024  W. Schwotzer

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

#ifndef REGISTR_INCLUDED
#define REGISTR_INCLUDED

#ifdef _WIN32

#include "misc1.h"
#include <string>
#include <map>


class BRegistry
{

private:

    LONG lastError;
    HKEY hKey;

    BRegistry();

public:
    ~BRegistry();

    BRegistry(const BRegistry &regKey, const std::string &subKey);
    BRegistry(HKEY p_hKey);
    bool isOpened() const;
    LONG GetLastError() const;
    LONG Delete();
    LONG SetValue(const std::string &name, const std::string &value);
    LONG SetValue(const std::string &name, const int value);
    LONG SetValue(const std::string &name, const BYTE *value, int size);
    LONG GetValue(const std::string &name, std::string &value);
    LONG GetValue(const std::string &name, int &value);
    LONG GetValues(const std::string &keyPrefix,
            std::map<std::string, std::string> &values);
    LONG DeleteValue(const std::string &name);
    // implicit type conversions !
    operator HKEY() const ;

    static BRegistry classesRoot;
    static BRegistry currentUser;
    static BRegistry localMachine;
    static BRegistry users;
};

inline bool BRegistry::isOpened() const
{
    return hKey != nullptr;
};
inline LONG BRegistry::GetLastError() const
{
    return lastError;
};
inline BRegistry::operator HKEY() const
{
    return hKey;
};

#endif // #ifdef _WIN32
#endif // BREGISTR_INCLUDED
