/*
    fclipbrd.h


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


#ifndef _FCLIPBRD_INCLUDED_
#define _FCLIPBRD_INCLUDED_

#include "misc1.h"
#include <wx/list.h>
#include "bobserv.h"
#include "filecont.h"
#include "bsingle.h"

WX_DECLARE_LIST(wxString, FlexFileList);

class FlexFileClipboard;

typedef BSingleton<FlexFileClipboard> SFlexFileClipboard;

class FlexFileClipboard : public BObserver
{

    friend FlexFileClipboard &SFlexFileClipboard::Instance();

public:
    virtual ~FlexFileClipboard();
    void SetEmpty() const;
    void SetFiles(const FlexFileList &newFileList) const;
    const FlexFileList &GetFiles() const
    {
        return m_fileList;
    };
    void SetSourceContainer(FileContainerIf *container) const
    {
        m_container = container;
    };
    FileContainerIf *GetSourceContainer() const
    {
        return m_container;
    };
    virtual void UpdateFrom(const void *pObject);

private:
    FlexFileClipboard();
    mutable FlexFileList    m_fileList;
    mutable FileContainerIf *m_container;
};

#endif // #ifndef _FCLIPBRD_INCLUDED_

