/*
    fdfdnd.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2019  W. Schwotzer

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

#include "warnoff.h"
// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    // Include your minimal set of headers here, or wx.h
    #include <wx/wx.h>
#endif
#include "warnon.h"

#include "misc1.h"

#ifdef wxUSE_DRAG_AND_DROP

#include "fdpframe.h"
#include "fdfdnd.h"
#include <vector>
#include <algorithm>

#ifndef __WXMOTIF__
FileDropTarget::FileDropTarget(FlexParentFrame *pOwner) : m_pOwner(pOwner)
{
}

bool FileDropTarget::OnDropFiles(wxCoord, wxCoord,
                                 const wxArrayString &fileNames)
{
    for (size_t i = 0; i < fileNames.GetCount(); ++i)
    {
        if (!m_pOwner->OpenContainer(fileNames.Item(i).ToUTF8().data()))
        {
            break;
        }
    }
    
    return true;
}

wxDragResult FileDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult)
{
    if (!GetData())
    {
        return wxDragNone;
    }

    wxFileDataObject *dobj = (wxFileDataObject *)m_dataObject;
    const wxArrayString fileNames = dobj->GetFilenames();
    bool success = true;

    for (size_t i = 0; success && (i < fileNames.GetCount()); ++i)
    {
        static std::vector<std::string> supportedExtensions = {
            ".dsk", ".flx", ".wta"
        };

        std::string filename(fileNames.Item(i).ToUTF8().data());
        std::string fileExtension = filename.substr(filename.size() - 4);

        std::transform(fileExtension.begin(), fileExtension.end(),
                       fileExtension.begin(), ::tolower);
        success &= (std::find(
                    supportedExtensions.begin(),
                    supportedExtensions.end(),
                    fileExtension) != supportedExtensions.end());
    }

    if (success)
    {
        return OnDropFiles(x, y, fileNames) ? wxDragCopy : wxDragNone;
    }

    return wxDragNone;
}

wxDragResult FileDropTarget::GetDefaultAction()
{
    return wxDragCopy;
}
#endif
#endif

