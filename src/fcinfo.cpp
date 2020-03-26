/*
    fcinfo.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2020  W. Schwotzer

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

#include <stdio.h>
#include <string.h>

#include "fcinfo.h"
#include "filecntb.h"   // needed for containertypes

#ifdef _
    #undef _
#endif

#define _(p) p


FlexContainerInfo::FlexContainerInfo() :
    sectors(0),
    tracks(0),
    number(0U),
    type(0),
    free(0),
    totalSize(0),
    attributes(0),
    is_flex_format(false)
{
} // FlexContainerInfo

FlexContainerInfo::~FlexContainerInfo()
{
}

const std::string FlexContainerInfo::GetTypeString() const
{
    std::string str;

    if (type & TYPE_CONTAINER)
    {
        if (type & TYPE_DSK_CONTAINER)
        {
            str = _("file container, DSK format");
        }
        else if (type & TYPE_FLX_CONTAINER)
        {
            str = _("file container, FLX format");
        }
        else
        {
            str = _("file container");
        }
    }
    else if (type & TYPE_DIRECTORY)
    {
        str = _("directory");
    }
    else
    {
        str = _("Unknown type");
    }

    return str;
}

