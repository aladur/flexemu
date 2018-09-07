/*
    ffilebuf.h


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

#ifndef FFILEBUF_INCLUDED
#define FFILEBUF_INCLUDED

#include "misc1.h"
#include "bmembuf.h"
#include "flexemu.h"
#include "bdate.h"
#include <memory>


typedef char FlexFileName[FLEX_FILENAME_LENGTH];
const DWord flexFileHeaderMagicNumber = 0xdeadbeaf;

// This is a POD data structure. It can be used to
// read or write FLEX files to and from the clipboard.
// A POD is needed here to simply copy it by memcpy().
struct tFlexFileHeader
{
    DWord magicNumber;
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

    int ConvertFromFlex();
    int ConvertToFlex();
    bool WriteToFile(const char *path) const;
#ifdef __GNUC__
    bool WriteToFile(int fd) const;
#endif
    bool ReadFromFile(const char *path);
    bool IsTextFile() const;
    bool IsFlexTextFile() const;
    bool IsExecutableFile() const;
    void CopyHeaderFrom(const tFlexFileHeader *from);
    bool CopyFrom(const Byte *from, unsigned int aSize,
                  unsigned int offset = 0);
    bool CopyTo(Byte *to, unsigned int aSize,
                unsigned int offset = 0, int stuffByte = -1) const;
    bool CopyTo(BMemoryBuffer &memory);
    void FillWith(const Byte pattern = 0);
    void Realloc(unsigned int newSize,
                 bool restoreContents = false);  // must NOT be virtual !!
    const Byte *GetBuffer(unsigned int offset = 0,
                          unsigned int bytes = 1) const;
    const tFlexFileHeader &GetHeader() const
    {
        return fileHeader;
    }
    void  SetDate(const BDate &date);
    void  SetDate(int day, int month, int year);
    void SetFilename(const char *name);
    void SetAdjustedFilename(const char *name);
    inline const char *GetFilename() const
    {
        return fileHeader.fileName;
    };
    inline unsigned int GetFileSize() const
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
        return fileHeader.attributes;
    };
    inline void SetAttributes(Byte attributes)
    {
        fileHeader.attributes = attributes;
    };
    inline int IsRandom() const
    {
        return (fileHeader.sectorMap != 0);
    }
    inline void SetSectorMap(int aSectorMap)
    {
        fileHeader.sectorMap = aSectorMap;
    }
    inline int GetSectorMap() const
    {
        return fileHeader.sectorMap;
    }
    const BDate GetDate() const;

private:
    void copyFrom(const FlexFileBuffer &src);
    unsigned int SizeOfFlexFile();
    unsigned int SizeOfFile();

    tFlexFileHeader fileHeader;
    std::unique_ptr<Byte[]> buffer;
};

#endif
