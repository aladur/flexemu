/*
    fmenufac.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2018  W. Schwotzer

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

#ifndef FMENUFAC_INCLUDED
#define FMENUFAC_INCLUDED

enum tFlxMenuId
{
    fFileMenuId,
    fEditMenuId,
    fExtrasMenuId,
    fContainerMenuId,
    fHelpMenuId
};

/*---------------------------------------------------------
 FlexMenuFactory
 simple factory class used for creating menus on the heap
-----------------------------------------------------------*/
class FlexMenuFactory
{
public:
    static wxMenu *CreateMenu(enum tFlxMenuId id);
};
#endif

