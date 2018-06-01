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

#ifndef __fddnd_h__
#define __fddnd_h__

#include <wx/defs.h>
#include <wx/dataobj.h>
#include <wx/dnd.h>

#ifdef wxUSE_DRAG_AND_DROP

#include <list>
#include "flexemu.h"

class FileContainerIf;
class FlexDiskListCtrl;
class FlexFileBuffer;

/*------------------------------------------------------
 FlexDnDFile
 a class containing a list of FlexFileBuffer object
 pointers.
 It can be used to do serialization/deserialization
 of a list of FLEX files
--------------------------------------------------------*/

extern wxString FlexFileFormatId;

typedef char FlexFileName[FLEX_FILENAME_LENGTH];
typedef std::list<FlexFileBuffer *> tFlexFileBufferArray;

class FlexDnDFiles
{
    struct tFlexDnDFile
    {
        DWord size;
        Word  attributes;
        Word  sectorMap;
        Word  day;
        Word  month;
        Word  year;
        FlexFileName fileName;
        char  data;
    };
public:
    FlexDnDFiles();
    virtual ~FlexDnDFiles();

    void ReadData(const Byte *p);
    size_t GetDataSize() const;
    void GetDataHere(Byte *buf) const;
    unsigned int GetFileCount() const
    {
        return fileList.size();
    };
    void Add(FlexFileBuffer *pFileBuffer);
    FlexFileBuffer &GetBuffer(unsigned int);
private:
    tFlexFileBufferArray fileList;
};

/*------------------------------------------------------
 FlexFileDataObject
 a data class used to drag and drop FLEX files
--------------------------------------------------------*/
class FlexFileDataObject : public wxCustomDataObject
{
public:
    FlexFileDataObject();
    void GetDataFrom(FlexDnDFiles &f);
    void SetDataTo(FlexDnDFiles &f);
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

/*------------------------------------------------------
 FileDropTarget
 a class needed for Drag & Drop functionality
 used to drop Text files
--------------------------------------------------------*/
class FileDropTarget : public wxFileDropTarget
{
public:
    FileDropTarget(FlexDiskListCtrl *pOwner) :
        m_pOwner(pOwner) { };
private:
    bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &);
    FileDropTarget(); // default constructor should not be used
    FlexDiskListCtrl *m_pOwner;
};
#endif

#endif
#endif

