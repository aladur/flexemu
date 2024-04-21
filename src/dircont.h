/*
    dircont.h


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

#ifndef DIRCONT_INCLUDED
#define DIRCONT_INCLUDED

#include "misc1.h"
#include <stdio.h>

#include "efiletim.h"
#include "filecont.h"
#include "fdirent.h"
#include "flexerr.h"
#include <string>


class FlexContainerInfo;
class FlexDirEntry;
class BDate;
class DirectoryContainerIteratorImp;


class DirectoryContainer : public FileContainerIf
{
    friend class DirectoryContainerIteratorImp; // corresponding iterator class

private:
    std::string directory;
    Byte attributes{};
    Word disk_number{};
    const FileTimeAccess &ft_access{};

public:
    DirectoryContainer() = delete;
    DirectoryContainer(const DirectoryContainer &) = delete;
    DirectoryContainer(DirectoryContainer &&) = delete;
    DirectoryContainer(const std::string &path,
                       const FileTimeAccess &fileTimeAccess);
    ~DirectoryContainer() override = default;

    DirectoryContainer &operator= (const DirectoryContainer &) = delete;
    DirectoryContainer &operator= (DirectoryContainer &&) = delete;

    static DirectoryContainer *Create(const std::string &directory,
                                      const std::string &name,
                                      int tracks,
                                      int sectors,
                                      const FileTimeAccess &fileTimeAccess,
                                      int fmt = TYPE_DSK_CONTAINER);

    // FileContainerIfBase interface declaration.
    bool IsWriteProtected() const override;
    bool GetInfo(FlexContainerInfo &info) const override;
    int GetContainerType() const override;
    std::string GetPath() const override;

    // FileContainerIf interface declaration (to be used within flexplorer).
    FileContainerIf *begin() override
    {
        return this;
    };
    FileContainerIf *end() const override
    {
        return nullptr;
    };
    bool FindFile(const char *fileName, FlexDirEntry &entry) override;
    bool DeleteFile(const char *wildcard) override;
    bool RenameFile(const char *oldName, const char *newName) override;
    bool SetAttributes(const char *wildcard, Byte setMask,
                       Byte clearMask = ~0) override;
    FlexFileBuffer ReadToBuffer(const char *fileName) override;
    bool WriteFromBuffer(const FlexFileBuffer &buffer,
                         const char *fileName = nullptr) override;
    bool FileCopy(const char *sourceName, const char *destName,
                  FileContainerIf &destination) override;
    std::string GetSupportedAttributes() const override;

private:
    FileContainerIteratorImpPtr IteratorFactory() override;
    static bool IsFlexFilename(const std::string &filename) ;
    bool SetDateTime(const char *fileName, const BDate &date,
                     const BTime &time);
    bool SetRandom(const char *fileName);
    void Initialize_header(Byte wp);

};

#endif // DIRCONT_INCLUDED
