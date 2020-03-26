/*
    fddnd.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2020  W. Schwotzer

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

#include "fddnd.h"
#include "ffilecnt.h"
#include "fdirent.h"
#include "fcinfo.h"
#include "fdlist.h"
#include "ffilebuf.h"


wxString FlexFileFormatId("FlexFileDataFormat");

FlexDnDFiles::FlexDnDFiles()
{
}

FlexDnDFiles::FlexDnDFiles(
        const std::string &pPath,
        const std::string &pDnsHostName) :
      path(pPath)
    , dnsHostName(pDnsHostName)
{
}

void FlexDnDFiles::ReadDataFrom(const Byte *buffer)
{
    const Byte *ptr = buffer;
    DWord count, index;
    DWord size;

    fileBuffers.clear();

    if (buffer == nullptr)
    {
        return;
    }

    size = *reinterpret_cast<const DWord *>(ptr);
    // Independent of the cpu architecture the
    // clipboard format uses big endian.
    size = fromBigEndian<DWord>(size);
    ptr += sizeof(size);
    dnsHostName = reinterpret_cast<const char *>(ptr);
    ptr += size;

    size = *reinterpret_cast<const DWord *>(ptr);
    size = fromBigEndian<DWord>(size);
    ptr += sizeof(size);
    path = reinterpret_cast<const char *>(ptr);
    ptr += size;

    count = *reinterpret_cast<const DWord *>(ptr);
    count = fromBigEndian<DWord>(count);
    ptr += sizeof(count);

    for (index = 0; index < count; ++index)
    {
        FlexFileBuffer fileBuffer;
        auto &fileHeader = *reinterpret_cast<const tFlexFileHeader *>(ptr);
        fileBuffer.CopyHeaderBigEndianFrom(fileHeader);
        ptr += sizeof(tFlexFileHeader);
        fileBuffer.CopyFrom(ptr, fileBuffer.GetFileSize());
        ptr += fileBuffer.GetFileSize();
        fileBuffers.push_back(fileBuffer);
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
    // Reserve memory for path size, dnsHostName size and file count
    size_t fileSize = 3 * sizeof(DWord);
    fileSize += path.size() + 1;
    fileSize += dnsHostName.size() + 1;

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
    DWord size, reversed;
    BDate date;

    if (buffer == nullptr)
    {
        return;
    }

    size = static_cast<DWord>(dnsHostName.size() + 1);
    // Independent of the cpu architecture the
    // clipboard format uses big endian byte order.
    reversed = toBigEndian<DWord>(size);
    memcpy(ptr, &reversed, sizeof(reversed));
    ptr += sizeof(reversed);
    memcpy(ptr, dnsHostName.c_str(), size);
    ptr += size;

    size = static_cast<DWord>(path.size() + 1);
    reversed = toBigEndian<DWord>(size);
    memcpy(ptr, &reversed, sizeof(reversed));
    ptr += sizeof(reversed);
    memcpy(ptr, path.c_str(), size);
    ptr += size;

    count = static_cast<DWord>(fileBuffers.size());
    count = toBigEndian<DWord>(count);
    memcpy(ptr, &count, sizeof(count));
    ptr += sizeof(count);

    for (auto iter = fileBuffers.cbegin(); iter != fileBuffers.cend(); ++iter)
    {
        auto header = iter->GetHeaderBigEndian();

        memcpy(ptr, &header, sizeof(tFlexFileHeader));
        ptr += sizeof(tFlexFileHeader);
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

void FlexDnDFiles::SetPath(const std::string &pPath)
{
    path = pPath;
}

std::string FlexDnDFiles::GetPath() const
{
    return path;
}

void FlexDnDFiles::SetDnsHostName(const std::string &pDnsHostName)
{
    dnsHostName = pDnsHostName;
}

std::string FlexDnDFiles::GetDnsHostName() const
{
    return dnsHostName;
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

    try {
        FlexDnDFiles files;

        ((FlexFileDataObject *)m_dataObject)->WriteDataTo(files);

        OnDropFiles(x, y, files);
    } catch (FlexException &ex)
    {
        wxMessageBox(ex.what(), _("FLEXplorer Error"),
                wxOK | wxCENTRE | wxICON_EXCLAMATION);
        return wxDragNone;
    }

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

