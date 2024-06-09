/*
    filecont.h

    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2024  W. Schwotzer

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

#ifndef FILECONT_INCLUDED
#define FILECONT_INCLUDED

#include "misc1.h"
#include <string>
#include "filecntb.h"
#include "ifilcnti.h"


class FlexCopyManager;
class FlexDirEntry;
class FlexContainerInfo;
class FlexFileBuffer;

//typedef int FileHdl;

// This interface describes a file oriented access to a FLEX disk image.
// Rename: FileContainerIf => IFlexDiskByFile
class IFlexDiskByFile : public IFlexDiskBase
{
    /* File oriented interface (to be used within flexplorer). */
public:
    virtual IFlexDiskByFile *begin() = 0;
    virtual IFlexDiskByFile *end() const = 0;
    virtual bool FindFile(const std::string &fileName, FlexDirEntry &entry) = 0;
    virtual bool DeleteFile(const std::string &wildcard) = 0;
    virtual bool RenameFile(const std::string &oldName,
                            const std::string &newName) = 0;
    virtual bool SetAttributes(const std::string &wildcard, Byte setMask,
                               Byte clearMask = ~0) = 0;
    virtual FlexFileBuffer ReadToBuffer(const std::string &fileName) = 0;
    virtual bool WriteFromBuffer(const FlexFileBuffer &buffer,
                                 const char *fileName = nullptr) = 0;
    virtual bool FileCopy(const std::string &sourceName,
                          const std::string &destName,
                          IFlexDiskByFile &destination) = 0;
    virtual std::string GetSupportedAttributes() const = 0;

    ~IFlexDiskByFile() override = default;

private:
    virtual IFlexDiskIteratorImpPtr IteratorFactory() = 0;
    friend class FlexDiskIterator;
    friend class FlexCopyManager;
};

#endif /* FILECONT_INCLUDED */
