/*
    bregistr.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2004 W. Schwotzer

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


#ifdef WIN32
#include <misc1.h>
#include "bregistr.h"

BRegistry BRegistry::classesRoot    = HKEY_CLASSES_ROOT;
BRegistry BRegistry::currentUser    = HKEY_CURRENT_USER;
BRegistry BRegistry::localMachine   = HKEY_LOCAL_MACHINE;
BRegistry BRegistry::users          = HKEY_USERS;

BRegistry::BRegistry(void) : lastError(0), hKey(NULL)
{
}

BRegistry::BRegistry(HKEY aHKey) :  lastError(0), hKey(aHKey)
{
}

BRegistry::BRegistry(const BRegistry &regKey, const char *subKey /* = NULL */)
{
    DWORD flags;

    lastError = RegCreateKeyEx(regKey, subKey, 0, "", 0, KEY_ALL_ACCESS, NULL,
                               &hKey, &flags);

    if (lastError != ERROR_SUCCESS)
    {
        hKey = NULL;
    }
}

BRegistry::~BRegistry()
{
    if (hKey != NULL)
    {
        lastError = RegCloseKey(hKey);
    }

    hKey = NULL;
}

LONG BRegistry::Delete()
{
    lastError = RegCloseKey(hKey);
    return lastError;
}

LONG BRegistry::SetValue(const char *name, const char *value)
{
    lastError = RegSetValueEx(hKey, name, 0, REG_SZ,
                              (CONST BYTE *)value, strlen(value) + 1);
    return lastError;
}

LONG BRegistry::SetValue(const char *name, const int value)
{
    lastError = RegSetValueEx(hKey, name, 0, REG_DWORD,
                              (CONST BYTE *)&value, sizeof(DWORD));
    return lastError;
}

LONG BRegistry::SetValue(const char *name, const BYTE *value, int size)
{
    lastError = RegSetValueEx(hKey, name, 0, REG_DWORD,
                              (CONST BYTE *)value, size);
    return lastError;
}

LONG BRegistry::GetValue(const char *name, std::string &value)
{
    char *str;
    DWORD aSize;
    DWORD type;

    if ((lastError = RegQueryValueEx(hKey, name, 0, &type, NULL,
                                     &aSize)) == ERROR_SUCCESS)
    {
        if ((str = new char [aSize]) != NULL)
        {
            lastError = RegQueryValueEx(hKey, name, 0, &type, (BYTE *)str,
                                        &aSize);

            if (lastError == ERROR_SUCCESS)
            {
                value = str;
            }
        }

        delete [] str;
    }

    return lastError;
}

LONG BRegistry::GetValue(const char *name, int *pvalue)
{
    DWORD aSize;
    DWORD type;

    aSize = sizeof(DWORD);
    lastError = RegQueryValueEx(hKey, name, 0, &type, (BYTE *)pvalue, &aSize);
    return lastError;
}

#endif
