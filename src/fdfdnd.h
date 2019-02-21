/*
    fdfdnd.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2019  W. Schwotzer

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

#ifndef FDFDND_INCLUDED
#define FDFDND_INCLUDED

#include <wx/defs.h>
#include <wx/dnd.h>

#ifdef wxUSE_DRAG_AND_DROP
#ifndef __WXMOTIF__

/*------------------------------------------------------
 FileDropTarget
 a class needed for Drag & Drop functionality
 used to drop FLEX container files
--------------------------------------------------------*/

class FlexParentFrame;

class FileDropTarget : public wxFileDropTarget
{
public:
    FileDropTarget(FlexParentFrame *pOwner);
    FileDropTarget() = delete;

    bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames);
    wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult defResult);

private:
    FlexParentFrame *m_pOwner;
};
#endif
#endif
#endif

