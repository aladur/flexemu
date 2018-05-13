/*
    fclipbrd.cpp


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

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#endif

#include "fclipbrd.h"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(FlexFileList)

template <> FlexFileClipboard *SFlexFileClipboard::instance = NULL;

FlexFileClipboard::FlexFileClipboard()
    : m_container(NULL)
{
}

FlexFileClipboard::~FlexFileClipboard()
{
    SetEmpty();
}

void FlexFileClipboard::SetEmpty(void) const
{
    m_fileList.DeleteContents(TRUE);
    m_container = NULL;
}

void FlexFileClipboard::SetFiles(const FlexFileList &newFileList) const
{
    FlexFileList::Node *node;

    SetEmpty();

    for (node = newFileList.GetFirst(); node; node = node->GetNext())
    {
        m_fileList.Append(new wxString(*node->GetData()));
    }
}

// if source container is to be deleted
// clipboard contents gets invalid

void FlexFileClipboard::UpdateFrom(const void *pObject)
{
    if (m_container == pObject)
    {
        SetEmpty();
    }
}

