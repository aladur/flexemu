/*
    ffilebuf.cpp


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

#include "misc1.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "bdate.h"
#include "ffilebuf.h"
#include "bfileptr.h"


FlexFileBuffer::FlexFileBuffer(int n /* = 0 */) :
    pBuffer(NULL), pDate(NULL), size(0), attributes(0), sectorMap(0)
{
    Realloc(n);
    memset(filename, 0, sizeof(filename));
}

FlexFileBuffer::FlexFileBuffer(const FlexFileBuffer &src) :
    pBuffer(NULL), pDate(NULL), size(0), attributes(0), sectorMap(0)

{
    copyFrom(src);
}

FlexFileBuffer &FlexFileBuffer::operator=(const FlexFileBuffer &lhs)
{
    if (&lhs != this)
    {
        copyFrom(lhs);

        if (lhs.pBuffer == NULL)
        {
            delete [] pBuffer;
            pBuffer = NULL;
        }

        if (lhs.pDate == NULL)
        {
            delete pDate;
            pDate = NULL;
        }
    }

    return *this;
}

void FlexFileBuffer::copyFrom(const FlexFileBuffer &src)
{
    if (src.pBuffer != NULL)
    {
        Byte *newBuffer = new Byte[src.size];
        memcpy(newBuffer, src.pBuffer, src.size);
        delete [] pBuffer;
        pBuffer    = newBuffer;
        size       = src.size;
    }

    if (src.pDate != NULL)
    {
        pDate = new BDate(*src.pDate);
    }

    strncpy(filename, src.filename, FLEX_FILENAME_LENGTH);
    attributes = src.attributes;
    sectorMap  = src.sectorMap;
}

FlexFileBuffer::~FlexFileBuffer()
{
    delete [] pBuffer;
    delete pDate;
}

const Byte *FlexFileBuffer::GetBuffer(unsigned int offset /* = 0*/,
                                      unsigned int bytes /* = 1 */) const
{
    if (offset + bytes > size)
    {
        return NULL;
    }

    return pBuffer + offset;
}

void FlexFileBuffer::SetFilename(const char *name)
{
    strncpy(filename, name, FLEX_FILENAME_LENGTH);
}

// reallocate the buffer with diffrent size
// buffer will be initialized to zero or
// optionally with a copy of the contents of the old buffer
void FlexFileBuffer::Realloc(unsigned int newSize,
                             bool restoreContents /* = false*/)
{
    Byte *newBuffer;

    if (newSize == 0)
    {
        return;
    }

    if (newSize <= size)
    {
        // dont allocate memory if buffer size decreases
        size = newSize;
        return;
    }

    newBuffer = new Byte[newSize];
    memset(newBuffer, 0, newSize);

    if (pBuffer && restoreContents)
    {
        memcpy(newBuffer, pBuffer, size);
    }

    delete [] pBuffer;
    pBuffer = newBuffer;
    size = newSize;
}

unsigned int FlexFileBuffer::SizeOfFlexFile(void)
{
    unsigned int count = 0;

    for (unsigned int i = 0; i < size; i++)
    {
        switch (pBuffer[i])
        {
            case 0x0d:
#ifndef _WIN32
                count += 1;
#else
                count += 2;
#endif
                break;

            case 0x09:
                if (i < size - 1)
                {
                    count += pBuffer[++i];
                }
                else
                {
                    count++;
                }

                break;

            case 0x0a:
            case 0x00:
                break;

            default:
                count++;
                break;
        }
    }

    return count;
}

int FlexFileBuffer::ConvertFromFlex(void)
{
    Byte *newBuffer;
    unsigned int newIndex, newSize;
    unsigned int count;

    if (!pBuffer || size == 0)
    {
        return 0;
    }

    newSize = SizeOfFlexFile();
    newBuffer = new Byte[ newSize ];
    newIndex = 0;

    for (unsigned int i = 0; i < size; i++)
    {
        Byte c = pBuffer[i];

        if (c > 0x0d)
        {
            newBuffer[newIndex++] = c;
        }
        else if (c == 0x0d)
        {
#ifndef _WIN32
            newBuffer[newIndex++] = 0x0a;
#else
            newBuffer[newIndex++] = 0x0d;
            newBuffer[newIndex++] = 0x0a;
#endif
        }
        else if (c == 0x09)
        {
            /* expand space compression */
            if (i < size - 1)
            {
                count = pBuffer[++i];
            }
            else
            {
                count = 1;
            }

            while (count--)
            {
                newBuffer[newIndex++] = ' ';
            }
        }
        else if (c != 0x00 && c != 0x0a)
        {
            newBuffer[newIndex++] = c;
        }
    } // for

    delete [] pBuffer;
    pBuffer = newBuffer;
    size = newSize;
    return size;
}

int FlexFileBuffer::ConvertToFlex(void)
{
    Byte            c;
    Byte            *newBuffer;
    int             newIndex, newSize;
    unsigned int    i, spaces;

    if (!pBuffer || size == 0)
    {
        return 0;
    }

    newSize = SizeOfFile();
    newBuffer = new Byte[ newSize ];
    newIndex = 0;
    spaces = 0;

    if (0)
    {
        for (i = 0; i < size; i++)
        {
            c = pBuffer[i];

            if (c > ' ')
            {
                newBuffer[newIndex++] = c;
            }
            else
            {
                if (c != ' ' && c != 0x09 && spaces)
                {
                    if (spaces > 1)
                    {
                        newBuffer[newIndex++] = 0x09;
                        newBuffer[newIndex++] = spaces;
                    }
                    else
                    {
                        newBuffer[newIndex++] = ' ';
                    }

                    spaces = 0;
                }

                if (c == ' ')
                {
                    // do space compression
                    if (++spaces == 127)
                    {
                        newBuffer[newIndex++] = 0x09;
                        newBuffer[newIndex++] = spaces;
                        spaces = 0;
                    }
                }
                else if (c == 0x09)
                {
                    // tab will be converted to 8 spaces
                    if (spaces >= 127 - 8)
                    {
                        newBuffer[newIndex++] = 0x09;
                        newBuffer[newIndex++] = spaces;
                        spaces -= 127 - 8;
                    }
                    else
                    {
                        spaces += 8;
                    }
                }
                else if (c == 0x0a)
                {
                    newBuffer[newIndex++] = 0x0d;
                }
                else if (c != 0x0d)
                {
                    newBuffer[newIndex++] = c;
                }
            }
        } // while
    }
    else
    {
        for (unsigned int i = 0; i < size; i++)
        {
            c = pBuffer[i];

            if (c == ' ' && (++spaces == 127))
            {
                /* do space compression */
                newBuffer[newIndex++] = 0x09;
                newBuffer[newIndex++] = spaces;
                spaces = 0;
            }
            else
            {
                if (spaces)
                {
                    newBuffer[newIndex++] = 0x09;
                    newBuffer[newIndex++] = spaces;
                    spaces = 0;
                }

                if (c > ' ')
                {
                    newBuffer[newIndex++] = c;
                }
                else if (c == 0x0a)
                {
                    newBuffer[newIndex++] = 0x0d;
                }
                else if (c == 0x09)
                {
                    newBuffer[newIndex++] = 0x09;
                    newBuffer[newIndex++] = 8;
                }

                /* other control chars than 0x09 and 0x0d will be ignored */
            }
        } /* for */
    }

    delete [] pBuffer;
    pBuffer = newBuffer;
    size = newSize;
    return size;
}

unsigned int FlexFileBuffer::SizeOfFile(void)
{
    unsigned int    count, spaces;

    if (!pBuffer || size == 0)
    {
        return 0;
    }

    count = spaces = 0;

    for (unsigned int i = 0; i < size; i++)
    {
        Byte c = pBuffer[i];

        if (c == ' ' && (++spaces == 127))
        {
            count += 2;
            spaces = 0;
        }
        else
        {
            if (spaces)
            {
                count += 2;
                spaces = 0;
            }

            if (c == 0x0a || c > ' ')
            {
                count++;
            }
            else
            {
                if (c == 0x09)
                {
                    count += 2;
                }

                /* other control chars than 0x09 and 0x0d will be ignored */
            }
        }
    } /* for */

    return count;
}

bool FlexFileBuffer::IsTextFile(void) const
{
    for (unsigned int i = 0; i < size; i++)
    {
        Byte c = pBuffer[i];

        if (c >= ' ' || c == 0x0a || c == 0x0d || c == 0x09)
        {
            continue;
        }

        return false;
    }

    return true;
}

bool FlexFileBuffer::IsFlexTextFile(void) const
{
    for (unsigned int i = 0; i < size; i++)
    {
        Byte c = pBuffer[i];

        if (c >= ' ' || c == 0x0a || c == 0x0d || c == 0x00)
        {
            continue;
        }

        if (c == 0x09 && i < size - 1)
        {
            i++;
            continue;
        }

        return false;
    }

    return true;
}

bool FlexFileBuffer::IsExecutableFile(void) const
{
    return false;
}

bool FlexFileBuffer::WriteToFile(const char *path) const
{
    BFilePtr fp(path, "wb");

    if (fp != NULL)
    {
        size_t blocks = fwrite(pBuffer, GetSize(), 1, fp);
        return (blocks == 1);
    }

    return false;
}

#ifdef __GNUC__
bool FlexFileBuffer::WriteToFile(int fd) const
{
    ssize_t bytes;

    bytes = write(fd, pBuffer, GetSize());
    return (bytes == -1 ? false : true);
}
#endif

bool FlexFileBuffer::ReadFromFile(const char *path)
{
    struct stat sbuf;

    if (!stat(path, &sbuf) && S_ISREG(sbuf.st_mode) && sbuf.st_size > 0)
    {
        BFilePtr fp(path, "rb");

        Realloc(sbuf.st_size);

        if (fp != NULL)
        {
            size_t blocks = fread(pBuffer, GetSize(), 1, fp);

            if (blocks == 1)
            {
                const char *pf;
                struct tm *lt;

                attributes = 0;
                sectorMap  = 0;
                pf = strrchr(path, PATHSEPARATOR);

                if (pf == NULL)
                {
                    pf = path;
                }
                else
                {
                    ++pf;
                }

                SetAdjustedFilename(pf);
                delete pDate;
                lt = localtime(&(sbuf.st_mtime));
                pDate = new BDate(lt->tm_mday,
                                  lt->tm_mon + 1, lt->tm_year + 1900);
                return true;
            }
        }
    }

    return false;
}

// Adjust the file name so that it is
// conformous to 8.3
void FlexFileBuffer::SetAdjustedFilename(const char *afileName)
{
    const char *p, *pe;

    memset(filename, '\0', FLEX_FILENAME_LENGTH);
    pe = strrchr(afileName, '.');
    strncpy(filename, afileName, 8);
    p = strrchr(filename, '.');

    if (p != NULL)
    {
        *(const_cast<char *>(p)) = '\0';
    }

    if (pe != NULL)
    {
        char ext[5];

        memset(ext, '\0', 5);
        strncpy(ext, pe, 4);
        strcat(filename, ext);
    }
}

bool FlexFileBuffer::CopyFrom(const Byte *from, unsigned int aSize,
                              unsigned int offset /* = 0 */)
{
    if (offset + aSize > size)
    {
        return false;
    }

    memcpy(pBuffer + offset, from, aSize);
    return true;
}

bool FlexFileBuffer::CopyTo(Byte *to, unsigned int aSize,
                            unsigned int offset /* = 0 */,
                            int stuffByte /* = -1 */) const
{
    if (offset + aSize > size)
    {
        if (stuffByte < 0 || offset >= size)
        {
            return false;
        }
        else
        {
            memset(to, stuffByte, aSize);
            memcpy(to, pBuffer + offset, size - offset);
            return true;
        }
    }

    memcpy(to, pBuffer + offset, aSize);
    return true;
}

void FlexFileBuffer::FillWith(const Byte pattern /* = 0 */)
{
    for (unsigned int i = 0; i < GetSize(); i++)
    {
        pBuffer[i] = pattern;
    }
}

void  FlexFileBuffer::SetDate(const BDate &date) const
{
    delete pDate;
    pDate = new BDate(date);
}

void  FlexFileBuffer::SetDate(int d, int m, int y) const
{
    delete pDate;
    pDate = new BDate(d, m, y);
}

const BDate &FlexFileBuffer::GetDate(void) const
{
    if (!pDate)
    {
        pDate = new BDate(BDate::Now());
    }

    return *pDate;
}

bool FlexFileBuffer::CopyTo(BMemoryBuffer &memory)
{
    const Byte *p;
    DWord address;
    DWord length;

    p = pBuffer;

    while ((size - (p - pBuffer)) >= 3)
    {
        switch (*(p++))
        {
            case 0x02: // memory contents
                if (size - (p - pBuffer) < 3)
                {
                    return false;
                }

                address = *p << 8 | *(p + 1);
                p += 2;
                length = *(p++);

                if (size - (p - pBuffer) < length)
                {
                    return false;
                }

                memory.CopyFrom(p, length, address);
                p += length;
                break;

            case 0x16: // start address, ignore
                p += 2;
                break;

            case 0x00: // continue with next sector
                length = 252 - ((p - pBuffer) % 252);
                p += length;
                break;

            default:
                return false;
        }
    }

    return true;
}
