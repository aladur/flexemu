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


FlexFileBuffer::FlexFileBuffer(int n /* = 0 */)
{
    memset(&fileHeader, 0, sizeof(fileHeader));
    Realloc(n);
}

FlexFileBuffer::FlexFileBuffer(const FlexFileBuffer &src)
{
    copyFrom(src);
}

FlexFileBuffer::FlexFileBuffer(FlexFileBuffer &&src)
{
    if (&src != this)
    {
        buffer = std::move(src.buffer);
        memcpy(&fileHeader, &src.fileHeader, sizeof(fileHeader));
        memset(&src.fileHeader, 0, sizeof(src.fileHeader));
    }
}

FlexFileBuffer &FlexFileBuffer::operator=(const FlexFileBuffer &src)
{
    if (&src != this)
    {
        copyFrom(src);
    }

    return *this;
}

FlexFileBuffer &FlexFileBuffer::operator=(FlexFileBuffer &&src)
{
    if (&src != this)
    {
        buffer = std::move(src.buffer);
        memcpy(&fileHeader, &src.fileHeader, sizeof(fileHeader));
        memset(&src.fileHeader, 0, sizeof(src.fileHeader));
    }

    return *this;
}

void FlexFileBuffer::copyFrom(const FlexFileBuffer &src)
{
    if (src.buffer != nullptr)
    {
        auto new_buffer = std::unique_ptr<Byte[]>(
                              new Byte[src.fileHeader.size]);
        memcpy(new_buffer.get(), src.buffer.get(), src.fileHeader.size);
        buffer = std::move(new_buffer);
        memcpy(&fileHeader, &src.fileHeader, sizeof(fileHeader));
    }
    else
    {
        buffer.reset(nullptr);
        memset(&fileHeader, 0, sizeof(fileHeader));
    }
}

FlexFileBuffer::~FlexFileBuffer()
{
}

const Byte *FlexFileBuffer::GetBuffer(unsigned int offset /* = 0*/,
                                      unsigned int bytes /* = 1 */) const
{
    if (offset + bytes > fileHeader.size)
    {
        return nullptr;
    }

    return buffer.get() + offset;
}

void FlexFileBuffer::SetFilename(const char *name)
{
    strncpy(fileHeader.fileName, name, FLEX_FILENAME_LENGTH - 1);
    fileHeader.fileName[sizeof(fileHeader.fileName) - 1] = '\0';
}

// reallocate the buffer with different size
// buffer will be initialized to zero or
// optionally with a copy of the contents of the old buffer
void FlexFileBuffer::Realloc(unsigned int new_size,
                             bool restoreContents /* = false*/)
{
    Byte *new_buffer;

    if (new_size == 0)
    {
        return;
    }

    if (new_size <= fileHeader.size)
    {
        // dont allocate memory if buffer size decreases
        fileHeader.size = new_size;
        return;
    }

    new_buffer = new Byte[new_size];
    memset(new_buffer, 0, new_size);

    if (buffer != nullptr && restoreContents)
    {
        memcpy(new_buffer, buffer.get(), fileHeader.size);
    }

    buffer.reset(new_buffer);
    fileHeader.size = new_size;
}

unsigned int FlexFileBuffer::SizeOfFlexFile()
{
    unsigned int count = 0;

    if (!buffer || fileHeader.size == 0)
    {
        return 0;
    }

    for (unsigned int i = 0; i < fileHeader.size; i++)
    {
        switch (buffer[i])
        {
            case 0x0d:
#ifndef _WIN32
                count += 1;
#else
                count += 2;
#endif
                break;

            case 0x09:
                if (i < fileHeader.size - 1)
                {
                    count += buffer[++i];
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

int FlexFileBuffer::ConvertFromFlex()
{
    Byte *new_buffer;
    unsigned int new_index, new_size;
    unsigned int count;

    if (!buffer || fileHeader.size == 0)
    {
        return 0;
    }

    new_size = SizeOfFlexFile();
    new_buffer = new Byte[new_size];
    new_index = 0;

    for (unsigned int i = 0; i < fileHeader.size; i++)
    {
        Byte c = buffer[i];

        if (c > 0x0d)
        {
            new_buffer[new_index++] = c;
        }
        else if (c == 0x0d)
        {
#ifndef _WIN32
            new_buffer[new_index++] = 0x0a;
#else
            new_buffer[new_index++] = 0x0d;
            new_buffer[new_index++] = 0x0a;
#endif
        }
        else if (c == 0x09)
        {
            /* expand space compression */
            if (i < fileHeader.size - 1)
            {
                count = buffer[++i];
            }
            else
            {
                count = 1;
            }

            while (count--)
            {
                new_buffer[new_index++] = ' ';
            }
        }
        else if (c != 0x00 && c != 0x0a)
        {
            new_buffer[new_index++] = c;
        }
    } // for

    buffer.reset(new_buffer);
    fileHeader.size = new_size;

    return fileHeader.size;
}

int FlexFileBuffer::ConvertToFlex()
{
    Byte            c;
    Byte            *new_buffer;
    int             new_index, new_size;
    unsigned int    i, spaces;

    if (!buffer || fileHeader.size == 0)
    {
        return 0;
    }

    new_size = SizeOfFile();
    new_buffer = new Byte[new_size];
    new_index = 0;
    spaces = 0;

    if (0)
    {
        for (i = 0; i < fileHeader.size; i++)
        {
            c = buffer[i];

            if (c > ' ')
            {
                new_buffer[new_index++] = c;
            }
            else
            {
                if (c != ' ' && c != 0x09 && spaces)
                {
                    if (spaces > 1)
                    {
                        new_buffer[new_index++] = 0x09;
                        new_buffer[new_index++] = spaces;
                    }
                    else
                    {
                        new_buffer[new_index++] = ' ';
                    }

                    spaces = 0;
                }

                if (c == ' ')
                {
                    // do space compression
                    if (++spaces == 127)
                    {
                        new_buffer[new_index++] = 0x09;
                        new_buffer[new_index++] = spaces;
                        spaces = 0;
                    }
                }
                else if (c == 0x09)
                {
                    // tab will be converted to 8 spaces
                    if (spaces >= 127 - 8)
                    {
                        new_buffer[new_index++] = 0x09;
                        new_buffer[new_index++] = spaces;
                        spaces -= 127 - 8;
                    }
                    else
                    {
                        spaces += 8;
                    }
                }
                else if (c == 0x0a)
                {
                    new_buffer[new_index++] = 0x0d;
                }
                else if (c != 0x0d)
                {
                    new_buffer[new_index++] = c;
                }
            }
        } // while
    }
    else
    {
        for (unsigned int i = 0; i < fileHeader.size; i++)
        {
            c = buffer[i];

            if (c == ' ' && (++spaces == 127))
            {
                /* do space compression */
                new_buffer[new_index++] = 0x09;
                new_buffer[new_index++] = spaces;
                spaces = 0;
            }
            else
            {
                if (spaces)
                {
                    new_buffer[new_index++] = 0x09;
                    new_buffer[new_index++] = spaces;
                    spaces = 0;
                }

                if (c > ' ')
                {
                    new_buffer[new_index++] = c;
                }
                else if (c == 0x0a)
                {
                    new_buffer[new_index++] = 0x0d;
                }
                else if (c == 0x09)
                {
                    new_buffer[new_index++] = 0x09;
                    new_buffer[new_index++] = 8;
                }

                /* other control chars than 0x09 and 0x0d will be ignored */
            }
        } /* for */
    }

    buffer.reset(new_buffer);
    fileHeader.size = new_size;

    return fileHeader.size;
}

unsigned int FlexFileBuffer::SizeOfFile()
{
    unsigned int count, spaces;

    if (!buffer || fileHeader.size == 0)
    {
        return 0;
    }

    count = spaces = 0;

    for (unsigned int i = 0; i < fileHeader.size; i++)
    {
        Byte c = buffer[i];

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

bool FlexFileBuffer::IsTextFile() const
{
    for (unsigned int i = 0; i < fileHeader.size; i++)
    {
        Byte c = buffer[i];

        if (c >= ' ' || c == 0x0a || c == 0x0d || c == 0x09)
        {
            continue;
        }

        return false;
    }

    return true;
}

bool FlexFileBuffer::IsFlexTextFile() const
{
    for (unsigned int i = 0; i < fileHeader.size; i++)
    {
        Byte c = buffer[i];

        if (c >= ' ' || c == 0x0a || c == 0x0d || c == 0x00)
        {
            continue;
        }

        if (c == 0x09 && i < fileHeader.size - 1)
        {
            i++;
            continue;
        }

        return false;
    }

    return true;
}

bool FlexFileBuffer::IsExecutableFile() const
{
    return false;
}

bool FlexFileBuffer::WriteToFile(const char *path) const
{
    BFilePtr fp(path, "wb");

    if (fp != nullptr)
    {
        size_t blocks = fwrite(buffer.get(), GetSize(), 1, fp);
        return (blocks == 1);
    }

    return false;
}

#ifdef __GNUC__
bool FlexFileBuffer::WriteToFile(int fd) const
{
    ssize_t bytes;

    bytes = write(fd, buffer.get(), GetSize());
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

        if (fp != nullptr)
        {
            size_t blocks = fread(buffer.get(), GetSize(), 1, fp);

            if (blocks == 1)
            {
                const char *pf;
                struct tm *lt;

                fileHeader.attributes = 0;
                fileHeader.sectorMap = 0;
                pf = strrchr(path, PATHSEPARATOR);

                if (pf == nullptr)
                {
                    pf = path;
                }
                else
                {
                    ++pf;
                }

                SetAdjustedFilename(pf);
                lt = localtime(&(sbuf.st_mtime));
                fileHeader.day = lt->tm_mday;
                fileHeader.month = lt->tm_mon + 1;
                fileHeader.year = lt->tm_year + 1900;
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

    memset(fileHeader.fileName, '\0', FLEX_FILENAME_LENGTH);
    pe = strrchr(afileName, '.');
    strncpy(fileHeader.fileName, afileName, 8);
    p = strrchr(fileHeader.fileName, '.');

    if (p != nullptr)
    {
        *(const_cast<char *>(p)) = '\0';
    }

    if (pe != nullptr)
    {
        char ext[5];

        memset(ext, '\0', 5);
        strncpy(ext, pe, 4);
        strcat(fileHeader.fileName, ext);
    }
}

bool FlexFileBuffer::CopyFrom(const Byte *from, unsigned int aSize,
                              unsigned int offset /* = 0 */)
{
    if (offset + aSize > fileHeader.size)
    {
        return false;
    }

    memcpy(&buffer[offset], from, aSize);
    return true;
}

bool FlexFileBuffer::CopyTo(Byte *to, unsigned int aSize,
                            unsigned int offset /* = 0 */,
                            int stuffByte /* = -1 */) const
{
    if (offset + aSize > fileHeader.size)
    {
        if (stuffByte < 0 || offset >= fileHeader.size)
        {
            return false;
        }
        else
        {
            memset(to, stuffByte, aSize);
            memcpy(to, &buffer[offset], fileHeader.size - offset);
            return true;
        }
    }

    memcpy(to, &buffer[offset], aSize);
    return true;
}

void FlexFileBuffer::FillWith(const Byte pattern /* = 0 */)
{
    for (unsigned int i = 0; i < GetSize(); i++)
    {
        buffer[i] = pattern;
    }
}

void FlexFileBuffer::SetDate(const BDate &new_date)
{
    fileHeader.day = new_date.GetDay();
    fileHeader.month = new_date.GetMonth();
    fileHeader.year = new_date.GetYear();
}

void FlexFileBuffer::SetDate(int day, int month, int year)
{
    fileHeader.day = day;
    fileHeader.month = month;
    fileHeader.year = year;
}

const BDate FlexFileBuffer::GetDate() const
{
    BDate date;

    date.Assign(
            fileHeader.day,
            fileHeader.month,
            fileHeader.year);

    return date;
}

bool FlexFileBuffer::CopyTo(BMemoryBuffer &memory)
{
    const Byte *p;
    DWord address;
    DWord length;

    p = buffer.get();

    while ((fileHeader.size - (p - buffer.get())) >= 3)
    {
        switch (*(p++))
        {
            case 0x02: // memory contents
                if (fileHeader.size - (p - buffer.get()) < 3)
                {
                    return false;
                }

                address = *p << 8 | *(p + 1);
                p += 2;
                length = *(p++);

                if (fileHeader.size - (p - buffer.get()) < length)
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
                length = 252 - ((p - buffer.get()) % 252);
                p += length;
                break;

            default:
                return false;
        }
    }

    return true;
}
