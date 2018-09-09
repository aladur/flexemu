/*
    flexdisk.h


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

#ifndef FLEXDISK_INCLUDED
#define FLEXDISK_INCLUDED

#include <wx/defs.h>
#include <wx/app.h>
#include "flexerr.h"


// FlexDisk Resource IDs
#define MDI_QUIT                  1
#define MDI_NEW_CONTAINER         2
#define MDI_ABOUT                 4
#define MDI_OPEN_CONTAINER        5
#define MDI_DELETE                6
#define MDI_RENAME                7
#define MDI_VIEW                 17
#define MDI_PROPERTIES            8
#define MDI_SET_WRITEPROTECT      9
#define MDI_SET_READPROTECT      10
#define MDI_SET_DELETEPROTECT    11
#define MDI_SET_CATALOGPROTECT   12
#define MDI_CLEAR_WRITEPROTECT   13
#define MDI_CLEAR_READPROTECT    14
#define MDI_CLEAR_DELETEPROTECT  15
#define MDI_CLEAR_CATALOGPROTECT 16
#define MDI_NEW_DIRECTORY        18
#define MDI_OPEN_DIRECTORY       19
#define MDI_CONTAINER_PROPERTIES 20
#define MDI_CONTAINER_CLOSE      21
#define MDI_COPY                 22
#define MDI_PASTE                23
#define MDI_FIND                 24
#define MDI_SELECTALL            25
#define MDI_DESELECTALL          26
#define MDI_OPTIONS              29


#define LIST_CTRL              1000

class FLEXplorer;

// Application class can be globally accessed by wxGetApp():

DECLARE_APP(FLEXplorer)

/*------------------------------------------------------
 FLEXplorer
 The main application class
 An explorer for any FLEX file or disk container
--------------------------------------------------------*/
class FLEXplorer: public wxApp
{
public:
    bool OnInit() override;
    int OnExit() override;

private:
    void ReadDefaultOptions();
    void WriteDefaultOptions();

    wxLocale m_locale;
};

#endif
