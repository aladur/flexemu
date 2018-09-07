/*
    filecont.h

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


class FileContainerIf : public FileContainerIfBase
{
    /* File oriented interface (to be used within flexdisk) */
public:
    virtual FileContainerIf *begin() = 0;
    virtual FileContainerIf *end() const = 0;
    virtual bool  CheckFilename(const char *fileName) const = 0;
    virtual bool  FindFile(const char *fileName, FlexDirEntry &entry) = 0;
    virtual bool  DeleteFile(const char *fileName) = 0;
    virtual bool  RenameFile(const char *oldName, const char *newName) = 0;
    virtual bool  SetAttributes(const char *fileName, Byte setMask = 0,
                                Byte clearMask = ~0) = 0;
    virtual FlexFileBuffer ReadToBuffer(const char *fileName) = 0;
    virtual bool  WriteFromBuffer(const FlexFileBuffer &buffer,
                                  const char *fileName = nullptr) = 0;
    virtual bool  FileCopy(const char *sourceName, const char *destName,
                           FileContainerIf &destination) = 0;
    virtual ~FileContainerIf() { };

private:
    virtual FileContainerIteratorImpPtr IteratorFactory() = 0;
    friend class FileContainerIterator;
    friend class FlexCopyManager;
};  /* class FileContainerIf */

#endif /* FILECONT_INCLUDED */
