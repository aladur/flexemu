/*
    bident.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2003-2018 W. Schwotzer

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

#include "bident.h"

BIdentifier::BIdentifier() : value(0), name("")
{
}

BIdentifier::BIdentifier(DWord aValue, const char *pName /* = nullptr */)
    : value(aValue), name(pName)
{
}

BIdentifier::BIdentifier(const BIdentifier &id) : value(id.value), name(id.name)
{
}

BIdentifier::~BIdentifier()
{
}

