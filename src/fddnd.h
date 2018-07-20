/*
    fddnd.h


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

#ifndef FDDND_INCLUDED
#define FDDND_INCLUDED

#include <wx/defs.h>
#include <wx/dataobj.h>
#include <wx/dnd.h>

#ifdef wxUSE_DRAG_AND_DROP

#include <vector>
#include "flexemu.h"
#include "ffilebuf.h"

class FileContainerIf;
class FlexDiskListCtrl;

/*------------------------------------------------------
 FlexDnDFile
 a class containing a list of FlexFileBuffer object
 pointers.
 It can be used to do serialization/deserialization
 of a list of FLEX files
--------------------------------------------------------*/

extern wxString FlexFileFormatId;

class FlexDnDFiles
{
public:
    FlexDnDFiles();
    virtual ~FlexDnDFiles();

    void ReadDataFrom(const Byte *buffer);
    void WriteDataTo(Byte *buffer) const;
    void Add(FlexFileBuffer &&fileBuffer);
    FlexFileBuffer &GetBufferAt(unsigned int);
    size_t GetFileSize() const;
    unsigned int GetFileCount() const
    {
        return fileBuffers.size();
    };

private:
    std::vector<FlexFileBuffer> fileBuffers;
};

/*------------------------------------------------------
 FlexFileDataObject
 a data class used to drag and drop FLEX files
--------------------------------------------------------*/
class FlexFileDataObject : public wxCustomDataObject
{
public:
    FlexFileDataObject();
    void ReadDataFrom(FlexDnDFiles &f);
    void WriteDataTo(FlexDnDFiles &f);
};

#ifndef __WXMOTIF__
/*------------------------------------------------------
 FlexFileDropTarget
 a class needed for Drag & Drop functionality
 used to drop FLEX files
--------------------------------------------------------*/
class FlexFileDropTarget : public wxDropTarget
{
public:
    FlexFileDropTarget(FlexDiskListCtrl *pOwner) :
        wxDropTarget(new FlexFileDataObject), m_pOwner(pOwner) { };
    wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def);

private:
    bool OnDropFiles(wxCoord x, wxCoord y, FlexDnDFiles &);
    FlexFileDropTarget(); // default constructor should not be used
    FlexDiskListCtrl *m_pOwner;
};
#endif

#endif
#endif

