/*
    fpattrui.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 2024-2025  W. Schwotzer

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


#include "fpattrui.h"

const FileAttributesUi::AttributeLabels_t
    &FileAttributesUi::GetAttributeLabels()
{
    static const FileAttributesUi::AttributeLabels_t attributeLabels =
    {
        "&Write Protect",
        "&Delete Protect",
        "&Read Protect",
        "&Catalog Protect",
    };

    return attributeLabels;
};


