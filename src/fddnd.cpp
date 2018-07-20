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
#include <sstream>


wxString FlexFileFormatId(wxT("FlexFileDataFormat"));

FlexDnDFiles::FlexDnDFiles()
{
}

void FlexDnDFiles::ReadDataFrom(const Byte *buffer)
{
    const Byte *ptr = buffer;
    tFlexFileHeader fileHeader;
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
        memcpy(&fileHeader, ptr, sizeof(tFlexFileHeader));
        if (fileHeader.magicNumber != flexFileHeaderMagicNumber)
        {
            std::stringstream stream;

            stream << std::hex << fileHeader.magicNumber;
            throw FlexException(FERR_INVALID_MAGIC_NUMBER, stream.str());
        }
        ptr += sizeof(tFlexFileHeader);
        FlexFileBuffer fileBuffer(fileHeader.fileSize);
        fileBuffer.CopyFrom(ptr, fileHeader.fileSize);
        fileBuffer.SetAttributes(fileHeader.attributes);
        fileBuffer.SetSectorMap(fileHeader.sectorMap);
        fileBuffer.SetFilename(fileHeader.fileName);
        fileBuffer.SetDate(fileHeader.day, fileHeader.month, fileHeader.year);
        fileBuffers.push_back(fileBuffer);
        ptr += fileHeader.fileSize;
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

size_t FlexDnDFiles::GetFileSize() const
{
    size_t fileSize = sizeof(DWord); // Contains the file byte size

    for (auto iter = fileBuffers.cbegin(); iter != fileBuffers.cend(); ++iter)
    {
        fileSize += iter->GetFileSize(); // Add byte size of each file
    }

    // Add size of header of each file
    fileSize += fileBuffers.size() * sizeof(tFlexFileHeader);

    return fileSize;
}

void FlexDnDFiles::WriteDataTo(Byte *buffer) const
{
    Byte *ptr = buffer;
    DWord count;
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
        memcpy(ptr, &iter->GetHeader(), sizeof(iter->GetHeader()));
        ptr += sizeof(iter->GetHeader());
        iter->CopyTo(ptr, iter->GetFileSize());
        ptr += iter->GetFileSize();
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
    size_t size = files.GetFileSize();
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

