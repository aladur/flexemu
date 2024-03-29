/*
    fcopyman.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2024  W. Schwotzer

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

#ifndef FCOPYMAN_INCLUDED
#define FCOPYMAN_INCLUDED

#include "misc1.h"
#include "flexerr.h"

class FileContainerIf;


class FlexCopyManager
{
public:
    static bool FileCopy(const char *srcName, const char *destName,
                         FileContainerIf &source, FileContainerIf &dest);
    static bool autoTextConversion;
}; // class FlexCopyManager

#endif // FCOPYMAN_INCLUDED

