/*
    fddnd.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2004  W. Schwotzer

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

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    // Include your minimal set of headers here, or wx.h
    #include <wx/wx.h>
#endif

#include "misc1.h"

#ifdef wxUSE_DRAG_AND_DROP

#include "fddnd.h"
#include "ffilecnt.h"
#include "fdirent.h"
#include "fcinfo.h"
#include "fdlist.h"
#include "ffilebuf.h"

wxString FlexFileFormatId(wxT("FlexFileDataFormat"));

FlexDnDFiles::FlexDnDFiles()
{
}

void FlexDnDFiles::ReadData(const Byte *buf)
{
    const Byte *p;
    tFlexDnDFile header;
    DWord count, i;
    tFlexFileBufferArray::iterator it;

    for (it = fileList.begin(); it != fileList.end(); ++it)
    {
        delete *it;
        *it = NULL;
    }

    fileList.clear();

    if (buf == NULL)
    {
        return;
    }

    p = buf;
    memcpy(&count, p, sizeof(count));
    p += sizeof(count);

    for (i = 0; i < count; i++)
    {
        memcpy(&header, p, sizeof(tFlexDnDFile) - 1);
        p += sizeof(tFlexDnDFile) - 1;
        FlexFileBuffer *pFileBuffer = new FlexFileBuffer(header.size);
        pFileBuffer->CopyFrom(p, pFileBuffer->GetSize());
        pFileBuffer->SetAttributes(header.attributes);
        pFileBuffer->SetSectorMap(header.sectorMap);
        pFileBuffer->SetFilename(header.fileName);
        pFileBuffer->SetDate(header.day, header.month, header.year);
        fileList.push_back(pFileBuffer);
        p += header.size;
    }
}

FlexFileBuffer &FlexDnDFiles::GetBuffer(unsigned int i)
{
    if (i >= GetFileCount())
    {
        return *(FlexFileBuffer *)(NULL);
    }

    tFlexFileBufferArray::iterator it;
    unsigned int count = i;

    for (it = fileList.begin(); count > 0; ++it, count--)
        ;

    return **it;
}

FlexDnDFiles::~FlexDnDFiles()
{
    tFlexFileBufferArray::iterator it;

    for (it = fileList.begin(); it != fileList.end(); ++it)
    {
        delete *it;
        *it = NULL;
    }
}

size_t FlexDnDFiles::GetDataSize() const
{
    tFlexFileBufferArray::const_iterator it;
    size_t dataSize = sizeof(DWord);

    for (it = fileList.begin(); it != fileList.end(); ++it)
    {
        dataSize += (*it)->GetSize();
    }

    dataSize += fileList.size() * (sizeof(tFlexDnDFile) - 1);
    return dataSize;
}

void FlexDnDFiles::GetDataHere(Byte *buf) const
{
    tFlexFileBufferArray::const_iterator it;
    DWord count;
    tFlexDnDFile header;
    BDate date;
    Byte *p = buf;

    if (buf == NULL)
    {
        return;
    }

    count = fileList.size();
    memcpy(p, &count, sizeof(count));
    p += sizeof(count);

    for (it = fileList.begin(); it != fileList.end(); ++it)
    {
        date              = (*it)->GetDate();
        header.day        = date.GetDay();
        header.month      = date.GetMonth();
        header.year       = date.GetYear();
        header.attributes = (*it)->GetAttributes();
        header.size       = (*it)->GetSize();
        header.sectorMap  = (*it)->GetSectorMap();
        memcpy(&header.fileName, (*it)->GetFilename(),
               FLEX_FILENAME_LENGTH);
        memcpy(p, &header, sizeof(tFlexDnDFile) - 1);
        p += sizeof(tFlexDnDFile) - 1;
        (*it)->CopyTo(p, header.size);
        p += header.size;
    }
}

// ATTENTION: pFileBuffer has to be created on the
// heap and will automatically be deleted
// in the destructor of FlexDnDFiles
void FlexDnDFiles::Add(FlexFileBuffer *pFileBuffer)
{
    if (pFileBuffer != NULL)
    {
        fileList.push_back(pFileBuffer);
    }
}

FlexFileDataObject::FlexFileDataObject()
{
    wxDataFormat dataFormat;

    dataFormat.SetId(FlexFileFormatId);
    SetFormat(dataFormat);
}

void FlexFileDataObject::SetDataTo(FlexDnDFiles &files)
{
    files.ReadData((const Byte *)GetData());
}

void FlexFileDataObject::GetDataFrom(FlexDnDFiles &files)
{
    size_t size = files.GetDataSize();
    Byte *pData = new Byte[size];
    files.GetDataHere(pData);
    SetData(size, pData);
    delete [] pData;
}

#ifndef __WXMOTIF__
wxDragResult FlexFileDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
    if (!GetData())
    {
        return wxDragNone;
    }

    FlexDnDFiles files;

    ((FlexFileDataObject *)m_dataObject)->SetDataTo(files);

    OnDropFiles(x, y, files);

    return def;
}

bool FlexFileDropTarget::OnDropFiles(wxCoord, wxCoord, FlexDnDFiles &files)
{
    if (m_pOwner != NULL)
    {
        return m_pOwner->PasteFrom(files);
    }

    return false;
}

bool FileDropTarget::OnDropFiles(wxCoord, wxCoord,
                                 const wxArrayString &fileNames)
{
    if (!GetData())
    {
        return wxDragNone;
    }

    FlexDnDFiles files;
    size_t i;

    for (i = 0; i < fileNames.GetCount(); i++)
    {
        FlexFileBuffer *pBuffer = new FlexFileBuffer;

        if (pBuffer->ReadFromFile(fileNames.Item(i).mb_str()))
        {
            if (pBuffer->IsTextFile())
            {
                pBuffer->ConvertToFlex();
            }

            files.Add(pBuffer);
        }
        else
        {
            delete pBuffer;
        }
    }

    if (m_pOwner != NULL)
    {
        return m_pOwner->PasteFrom(files);
    }

    return false;
}
#endif
#endif

