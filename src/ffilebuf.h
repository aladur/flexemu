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
#include <memory>


class FlexFileBuffer
{
public:
    FlexFileBuffer(int n = 0);
    FlexFileBuffer(const FlexFileBuffer &f);
    virtual ~FlexFileBuffer();

    FlexFileBuffer &operator=(const FlexFileBuffer &lhs);
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
    void  SetDate(const BDate &date) const;
    void  SetDate(int d, int m, int y) const;
    void SetFilename(const char *name);
    void SetAdjustedFilename(const char *name);
    inline const char *GetFilename() const
    {
        return filename;
    };
    inline unsigned int GetSize() const
    {
        return size;
    };
    inline bool IsEmpty() const
    {
        return size == 0;
    };
    inline operator const Byte *() const
    {
        return GetBuffer(0);
    };
    inline int GetAttributes() const
    {
        return attributes;
    };
    inline void SetAttributes(int attrs)
    {
        attributes = attrs;
    };
    inline int IsRandom() const
    {
        return (sectorMap != 0);
    }
    inline void SetSectorMap(int aSectorMap)
    {
        sectorMap = aSectorMap;
    }
    inline int GetSectorMap() const
    {
        return sectorMap;
    }
    const BDate &GetDate() const;

private:
    void copyFrom(const FlexFileBuffer &src);
    unsigned int SizeOfFlexFile();
    unsigned int SizeOfFile();
    char filename[FLEX_FILENAME_LENGTH];

    std::unique_ptr<Byte[]> buffer;
    mutable BDate date;
    unsigned int size;
    int attributes;
    Word sectorMap;
};

#endif
