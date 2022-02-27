/*
    ffilebuf.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2022  W. Schwotzer

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
#include <memory>
#include <functional>
#include <sstream>
#include <iomanip>
#include <array>


typedef char FlexFileName[FLEX_FILENAME_LENGTH];
const std::array<char,4> flexFileHeaderMagicNumber = {
    '\xde', '\xad', '\xbe', '\xaf'
};

// This is a POD data structure. It can be used to
// read or write FLEX files to and from the clipboard.
// A POD is needed here to simply copy it by memcpy().
// Independent of the cpu architecture the values
// have big-endian byte order.
struct tFlexFileHeader
{
    char magicNumber[4];
    DWord fileSize;
    Word  attributes;
    Word  sectorMap;
    Word  day;
    Word  month;
    Word  year;
    FlexFileName fileName;
};

class FlexFileBuffer
{
public:
    FlexFileBuffer();
    FlexFileBuffer(const FlexFileBuffer &src);
    FlexFileBuffer(FlexFileBuffer &&src);
    virtual ~FlexFileBuffer();

    FlexFileBuffer &operator=(const FlexFileBuffer &src);
    FlexFileBuffer &operator=(FlexFileBuffer &&src);

    void ConvertToTextFile();
    void ConvertToFlexTextFile();
    void ConvertToDumpFile(DWord bytesPerLine);
    bool WriteToFile(const char *path) const;
    bool ReadFromFile(const char *path);
    bool IsTextFile() const;
    bool IsFlexTextFile() const;
    bool IsFlexExecutableFile() const;
    void CopyHeaderBigEndianFrom(const tFlexFileHeader &from);
    bool CopyFrom(const Byte *from, DWord aSize, DWord offset = 0);
    bool CopyTo(Byte *to, DWord aSize,
                DWord offset = 0, int stuffByte = -1) const;
    void FillWith(const Byte pattern = 0);
    void Realloc(DWord newSize, bool restoreContents = false);
    const Byte *GetBuffer(DWord offset = 0, DWord bytes = 1) const;
    const tFlexFileHeader &GetHeader() const
    {
        return fileHeader;
    }
    tFlexFileHeader GetHeaderBigEndian() const;
    void SetDate(const BDate &date);
    void SetDate(int day, int month, int year);
    void SetFilename(const char *name);
    void SetAdjustedFilename(const char *name);
    inline const char *GetFilename() const
    {
        return fileHeader.fileName;
    };
    inline DWord GetFileSize() const
    {
        return fileHeader.fileSize;
    };
    inline bool IsEmpty() const
    {
        return !buffer || fileHeader.fileSize == 0;
    };
    inline operator const Byte *() const
    {
        if (IsEmpty())
        {
            return nullptr;
        }
        else
        {
            return GetBuffer(0);
        }
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
    inline void SetSectorMap(int aSectorMap)
    {
        fileHeader.sectorMap = static_cast<Word>(aSectorMap);
    }
    inline int GetSectorMap() const
    {
        return fileHeader.sectorMap;
    }
    const BDate GetDate() const;

private:
    void copyFrom(const FlexFileBuffer &src);
    void TraverseForTextFileConversion(std::function<void(char c)> fct) const;
    void TraverseForFlexTextFileConversion(std::function<void(char c)> fct)
         const;
    void TraverseForDumpFileConversion(DWord bytesPerLine,
                                       std::function<void(char c)> fct) const;
    DWord SizeOfConvertedTextFile() const;
    DWord SizeOfConvertedFlexTextFile() const;
    DWord SizeOfConvertedDumpFile(DWord bytesPerLine) const;

    tFlexFileHeader fileHeader;
    DWord capacity;
    std::unique_ptr<Byte[]> buffer;
};

#endif
