/*
    ffilebuf.h


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

#ifndef FFILEBUF_INCLUDED
#define FFILEBUF_INCLUDED

#include "misc1.h"
#include "filecntb.h"
#include "bdate.h"
#include "btime.h"
#include "efiletim.h"
#include "fdirent.h"
#include <memory>
#include <functional>
#include <sstream>
#include <iomanip>
#include <array>
#include <vector>
#include <string>
#include "warnoff.h"
#include <optional>
#include "warnon.h"


const std::array<char,4> flexFileHeaderMagicNumber = {
    // '\xde', '\xad', '\xbe', '\xaf' old magic number without hour, minute.
    '\xde', '\xad', '\xbe', '\xae'
};

/* POD are needed here for memcpy() to/from clipboard. */
/* NOLINTBEGIN(modernize-avoid-c-arrays) */
using FlexFileName = char[FLEX_FILENAME_LENGTH];

// This is a POD data structure. It can be used to
// read or write FLEX files to and from the clipboard.
// A POD is needed here to simply copy it by memcpy().
// Independent of the cpu architecture the values
// have big-endian byte order.
struct tFlexFileHeader
{
    char magicNumber[4];
    DWord fileSize;
    Word attributes;
    Word sectorMap;
    Word day;
    Word month;
    Word year;
    Word hour;
    Word minute;
    FlexFileName fileName;
};
/* NOLINTEND(modernize-avoid-c-arrays) */

class FlexFileBuffer
{
    enum class ReadCmdState : uint8_t
    {
        GetType,
        GetDataAddress,
        GetCount,
        GetData,
        GetStartAddress,
        GetNUL,
    };

public:
    FlexFileBuffer();
    FlexFileBuffer(const FlexFileBuffer &src);
    FlexFileBuffer(FlexFileBuffer &&src) noexcept;
    virtual ~FlexFileBuffer() = default;

    FlexFileBuffer &operator=(const FlexFileBuffer &src);
    FlexFileBuffer &operator=(FlexFileBuffer &&src) noexcept;

    void ConvertToTextFile();
    void ConvertToFlexTextFile();
    void ConvertToDumpFile(DWord bytesPerLine);
    bool WriteToFile(const std::string &path,
            FileTimeAccess fileTimeAccess, bool doRandomCheck = false) const;
    bool ReadFromFile(const std::string &path, FileTimeAccess fileTimeAccess,
            bool doRandomCheck = false);
    bool IsTextFile() const;
    bool IsFlexTextFile() const;
    bool IsFlexExecutableFile() const;
    void CopyHeaderBigEndianFrom(const tFlexFileHeader &src);
    bool CopyFrom(const Byte *source, DWord size, DWord offset = 0);
    bool CopyTo(Byte *target, DWord size,
                DWord offset = 0,
                std::optional<Byte> stuffByte = std::nullopt) const;
    void FillWith(Byte pattern = '\0');
    void Realloc(DWord newSize, bool restoreContents = false);
    const Byte *GetBuffer(DWord offset = 0, DWord bytes = 1) const;
    tFlexFileHeader GetHeaderBigEndian() const;
    void SetDateTime(const BDate &date, const BTime &time);
    void SetFilename(const std::string &name);
    inline std::string GetFilename() const
    {
        return fileHeader.fileName;
    };
    inline DWord GetFileSize() const
    {
        return fileHeader.fileSize;
    };
    inline bool IsEmpty() const
    {
        return buffer.empty() || fileHeader.fileSize == 0;
    };
    inline explicit operator const Byte *() const
    {
        if (IsEmpty())
        {
            return nullptr;
        }

        return GetBuffer(0);
    };
    inline Byte GetAttributes() const
    {
        return static_cast<Byte>(fileHeader.attributes);
    };
    inline void SetAttributes(Byte attributes)
    {
        fileHeader.attributes = attributes;
    };
    inline bool IsRandom() const
    {
        return (fileHeader.sectorMap != 0);
    }
    inline void SetSectorMap(int sectorMap)
    {
        fileHeader.sectorMap = static_cast<Word>(sectorMap);
    }
    inline int GetSectorMap() const
    {
        return fileHeader.sectorMap;
    }
    BDate GetDate() const;
    BTime GetTime() const;
    FlexDirEntry GetDirEntry() const;

private:
    void SetAdjustedFilename(const std::string &name);
    void copyFrom(const FlexFileBuffer &src);
    void TraverseForTextFileConversion(
            const std::function<void(Byte c)>& fct) const;
    void TraverseForFlexTextFileConversion(
            const std::function<void(Byte c)>& fct) const;
    void TraverseForDumpFileConversion(
            DWord bytesPerLine,
            const std::function<void(Byte c)>& fct) const;
    DWord SizeOfConvertedTextFile() const;
    DWord SizeOfConvertedFlexTextFile() const;
    DWord SizeOfConvertedDumpFile(DWord bytesPerLine) const;

    tFlexFileHeader fileHeader{};
    std::vector<Byte> buffer;
};

#endif
