/*
    fddnd.h


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 1998-2026  W. Schwotzer

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

#include "typedefs.h"
#include "ffilebuf.h"
#include <string>
#include <vector>

class IFlexDiskByFile;
class FlexDiskListCtrl;

/*------------------------------------------------------
 FlexDnDFiles
 a class containing a vector of FlexFileBuffer objects.
 It can be used to do serialization/deserialization
 of a list of FLEX files.
--------------------------------------------------------*/

class FlexDnDFiles
{
public:
    FlexDnDFiles() = default;
    FlexDnDFiles(std::string p_path, std::string p_dnsHostName);

    void ReadDataFrom(const Byte *buffer);
    void WriteDataTo(Byte *buffer) const;
    std::string GetPath() const;
    std::string GetDnsHostName() const;
    void Add(FlexFileBuffer &&fileBuffer);
    FlexFileBuffer &GetBufferAt(unsigned int index);
    DWord GetFileSize() const;
    unsigned int GetFileCount() const
    {
        return static_cast<unsigned int>(fileBuffers.size());
    };

private:
    std::string path;
    std::string dnsHostName;
    std::vector<FlexFileBuffer> fileBuffers;
};
#endif

