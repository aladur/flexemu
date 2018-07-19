/*
    fddnd.cpp


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

void FlexDnDFiles::ReadDataFrom(const Byte *buffer)
{
    const Byte *ptr = buffer;
    tFlexFileHeader header;
    DWord count, index;

    fileBuffers.clear();

    if (buffer == nullptr)
    {
        return;
    }

    memcpy(&count, ptr, sizeof(count));
    ptr += sizeof(count);

    for (index = 0; index < count; ++index)
    {
        memcpy(&header, ptr, sizeof(tFlexFileHeader));
        ptr += sizeof(tFlexFileHeader);
        FlexFileBuffer fileBuffer(header.size);
        fileBuffer.CopyFrom(ptr, header.size);
        fileBuffer.SetAttributes(header.attributes);
        fileBuffer.SetSectorMap(header.sectorMap);
        fileBuffer.SetFilename(header.fileName);
        fileBuffer.SetDate(header.day, header.month, header.year);
        fileBuffers.push_back(fileBuffer);
        ptr += header.size;
    }
}

FlexFileBuffer &FlexDnDFiles::GetBufferAt(unsigned int index)
{
    if (index >= fileBuffers.size())
    {
        throw FlexException(FERR_WRONG_PARAMETER);
    }

    return fileBuffers[index];
}

FlexDnDFiles::~FlexDnDFiles()
{
    fileBuffers.clear();
}

size_t FlexDnDFiles::GetDataSize() const
{
    size_t dataSize = sizeof(DWord); // Contains the file count

    for (auto iter = fileBuffers.cbegin(); iter != fileBuffers.cend(); ++iter)
    {
        dataSize += iter->GetSize(); // Add size of each file
    }

    // Add size of header (minus one for char data) of each file
    dataSize += fileBuffers.size() * sizeof(tFlexFileHeader);

    return dataSize;
}

void FlexDnDFiles::WriteDataTo(Byte *buffer) const
{
    Byte *ptr = buffer;
    DWord count;
    tFlexFileHeader header;
    BDate date;

    if (buffer == nullptr)
    {
        return;
    }

    count = fileBuffers.size();
    memcpy(ptr, &count, sizeof(count));
    ptr += sizeof(count);

    for (auto iter = fileBuffers.cbegin(); iter != fileBuffers.cend(); ++iter)
    {
        date = iter->GetDate();
        header.day = date.GetDay();
        header.month = date.GetMonth();
        header.year = date.GetYear();
        header.attributes = iter->GetAttributes();
        header.size = iter->GetSize();
        header.sectorMap  = iter->GetSectorMap();
        memcpy(&header.fileName, iter->GetFilename(), FLEX_FILENAME_LENGTH);
        memcpy(ptr, &header, sizeof(tFlexFileHeader));
        ptr += sizeof(tFlexFileHeader);
        iter->CopyTo(ptr, header.size);
        ptr += header.size;
    }
}

// ATTENTION: fileBuffer is an rvalue and moved into
// the fileBuffers.
void FlexDnDFiles::Add(FlexFileBuffer &&fileBuffer)
{
    fileBuffers.push_back(std::move(fileBuffer));
}

FlexFileDataObject::FlexFileDataObject()
{
    wxDataFormat dataFormat;

    dataFormat.SetId(FlexFileFormatId);
    SetFormat(dataFormat);
}

void FlexFileDataObject::WriteDataTo(FlexDnDFiles &files)
{
    files.ReadDataFrom((const Byte *)GetData());
}

void FlexFileDataObject::ReadDataFrom(FlexDnDFiles &files)
{
    size_t size = files.GetDataSize();
    auto buffer = std::unique_ptr<Byte[]>(new Byte[size]);
    files.WriteDataTo(buffer.get());
    SetData(size, buffer.get());
}

#ifndef __WXMOTIF__
wxDragResult FlexFileDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
    if (!GetData())
    {
        return wxDragNone;
    }

    FlexDnDFiles files;

    ((FlexFileDataObject *)m_dataObject)->WriteDataTo(files);

    OnDropFiles(x, y, files);

    return def;
}

bool FlexFileDropTarget::OnDropFiles(wxCoord, wxCoord, FlexDnDFiles &files)
{
    if (m_pOwner != nullptr)
    {
        return m_pOwner->PasteFrom(files);
    }

    return false;
}
#endif
#endif

