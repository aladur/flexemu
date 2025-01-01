/*
    dircont.h


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 1998-2025  W. Schwotzer

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
#include "efiletim.h"
#include "filecntb.h"
#include "filecont.h"
#include "fdirent.h"
#include "flexerr.h"
#include "rndcheck.h"
#include <string>


class FlexDiskAttributes;
class FlexDirEntry;
class BDate;
class FlexDirectoryDiskIteratorImp;

// class FlexDirectoryDiskByFile implements a file oriented access
// to a FLEX disk by mapping a host directory emulating a FLEX disk.
//
// Rename: DirectoryContainer => FlexDirectoryDiskByFile.
class FlexDirectoryDiskByFile : public IFlexDiskByFile
{
    friend class FlexDirectoryDiskIteratorImp; // corresponding iterator class

private:
    std::string directory;
    RandomFileCheck randomFileCheck;
    Byte attributes{};
    Word disk_number{};
    const FileTimeAccess &ft_access{};

public:
    FlexDirectoryDiskByFile() = delete;
    FlexDirectoryDiskByFile(const FlexDirectoryDiskByFile &) = delete;
    FlexDirectoryDiskByFile(FlexDirectoryDiskByFile &&) = delete;
    FlexDirectoryDiskByFile(const std::string &path,
                            const FileTimeAccess &fileTimeAccess);
    ~FlexDirectoryDiskByFile() override = default;

    FlexDirectoryDiskByFile &operator= (
            const FlexDirectoryDiskByFile &) = delete;
    FlexDirectoryDiskByFile &operator= (FlexDirectoryDiskByFile &&) = delete;

    static FlexDirectoryDiskByFile *Create(const std::string &directory,
            const std::string &name, int tracks, int sectors,
            const FileTimeAccess &fileTimeAccess,
            DiskType disk_type);

    // IFlexDiskBase interface declaration.
    bool IsWriteProtected() const override;
    bool GetDiskAttributes(FlexDiskAttributes &diskAttributes) const override;
    DiskType GetFlexDiskType() const override;
    DiskOptions GetFlexDiskOptions() const override;
    std::string GetPath() const override;

    // IFlexDiskByFile interface declaration (to be used within flexplorer).
    IFlexDiskByFile *begin() override
    {
        return this;
    };
    IFlexDiskByFile *end() const override
    {
        return nullptr;
    };
    bool FindFile(const std::string &wildcard, FlexDirEntry &dirEntry) override;
    bool DeleteFile(const std::string &wildcard) override;
    bool RenameFile(const std::string &oldName,
                    const std::string &newName) override;
    bool SetAttributes(const std::string &wildcard, unsigned setMask,
                       unsigned clearMask = ~0U) override;
    FlexFileBuffer ReadToBuffer(const std::string &fileName) override;
    bool WriteFromBuffer(const FlexFileBuffer &buffer,
                         const char *fileName = nullptr) override;
    bool FileCopy(const std::string &sourceName, const std::string &destName,
                  IFlexDiskByFile &destination) override;
    std::string GetSupportedAttributes() const override;

private:
    IFlexDiskIteratorImpPtr IteratorFactory() override;
    bool SetDateTime(const std::string &fileName, const BDate &date,
                     const BTime &time);
    void Initialize_header();
};

#endif // DIRCONT_INCLUDED
