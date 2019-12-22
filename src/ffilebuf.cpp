/*
    ffilebuf.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2019  W. Schwotzer

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
#include "flexerr.h"
#include <sstream>
#include <algorithm>


// The format of a FLEX text file is described in the
// FLEX Advanced Programmer's Guide in Chapter
// DESCRIPTION OF A TEXT FILE.

FlexFileBuffer::FlexFileBuffer() : capacity(0)
{
    memset(&fileHeader, 0, sizeof(fileHeader));
    memcpy(fileHeader.magicNumber,
           flexFileHeaderMagicNumber.data(),
           flexFileHeaderMagicNumber.size());
}

FlexFileBuffer::FlexFileBuffer(const FlexFileBuffer &src)
{
    copyFrom(src);
}

FlexFileBuffer::FlexFileBuffer(FlexFileBuffer &&src)
{
    buffer = std::move(src.buffer);
    capacity = src.capacity;
    memcpy(&fileHeader, &src.fileHeader, sizeof(fileHeader));
    memset(&src.fileHeader, 0, sizeof(src.fileHeader));
    src.capacity = 0;
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
        capacity = src.capacity;
        memcpy(&fileHeader, &src.fileHeader, sizeof(fileHeader));
        memset(&src.fileHeader, 0, sizeof(src.fileHeader));
        src.capacity = 0;
    }

    return *this;
}

void FlexFileBuffer::copyFrom(const FlexFileBuffer &src)
{
    if (src.buffer != nullptr)
    {
        auto new_buffer = std::unique_ptr<Byte[]>(
                              new Byte[src.fileHeader.fileSize]);
        capacity = src.fileHeader.fileSize;
        memcpy(new_buffer.get(), src.buffer.get(), src.fileHeader.fileSize);
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

const Byte *FlexFileBuffer::GetBuffer(DWord offset /* = 0*/,
                                      DWord bytes /* = 1 */) const
{
    if (offset + bytes > fileHeader.fileSize)
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

// Reallocate the buffer with a different size.
// Buffer will be initialized to zero or
// optionally with a copy of the contents of the old buffer.
void FlexFileBuffer::Realloc(DWord new_size,
                             bool restoreContents /* = false*/)
{
    Byte *new_buffer;

    if (new_size == 0)
    {
        return;
    }

    if (new_size <= capacity)
    {
        // Don't allocate memory if buffer capacity decreases.
        if (new_size > fileHeader.fileSize)
        {
            memset(&buffer[fileHeader.fileSize], 0,
                   new_size - fileHeader.fileSize);
        }
        fileHeader.fileSize = new_size;
        return;
    }

    new_buffer = new Byte[new_size];
    memset(new_buffer, 0, new_size);

    if (buffer != nullptr && restoreContents)
    {
        memcpy(new_buffer, buffer.get(), fileHeader.fileSize);
    }

    buffer.reset(new_buffer);
    fileHeader.fileSize = new_size;
    capacity = new_size;
}

// Traverse through a given FLEX text file and call a function for each
// converted character for converting to a host operating system text file.
void FlexFileBuffer::TraverseForTextFileConversion(
        std::function<void(char c)> fct) const
{
    for (DWord index = 0; index < fileHeader.fileSize; index++)
    {
        Byte c = buffer[index];

        if (c >= ' ')
        {
            fct(c);
        }
        else if (c == 0x0d)
        {
            // Convert ASCII CR, the FLEX text file end of line character
            // into a new line (depending on the operating system).
#ifdef _WIN32
            fct(0x0d);
#endif
            fct(0x0a);
        }
        else if (c == 0x09)
        {
            DWord spaces;

            // Expand space compression.
            if (index < fileHeader.fileSize - 1)
            {
                spaces = buffer[++index];
            }
            else
            {
                spaces = 1;
            }

            while (spaces--)
            {
                fct(' ');
            }
        }

        // Other control characters than ASCII TAB, ASCII CR are ignored.
    } // for
}

// Estimate the needed buffer size after converting
// a given FLEX text file into a text file on the host operating
// system.
// Returns the estimated file size in byte.
DWord FlexFileBuffer::SizeOfConvertedTextFile() const
{
    DWord count = 0;

    if (!buffer)
    {
        return count;
    }

    TraverseForTextFileConversion([&count](char)
        {
            count++;
        });

    return count;
}

// Convert a FLEX text file into a text file on the host operating
// system.
// Replace the buffer contents by the converted file contents.
void FlexFileBuffer::ConvertToTextFile()
{
    if (!buffer || fileHeader.fileSize == 0)
    {
        return;
    }

    DWord new_size = SizeOfConvertedTextFile();
    Byte *new_buffer = new Byte[new_size];
    DWord new_index = 0;

    TraverseForTextFileConversion([&new_buffer, &new_index](char c)
        {
            new_buffer[new_index++] = c;
        });

    buffer.reset(new_buffer);
    fileHeader.fileSize = new_size;
    capacity = new_size;
}

// Traverse through a given host operating system text file and call a function
// for each converted character for converting to a FLEX text file.
void FlexFileBuffer::TraverseForFlexTextFileConversion(
        std::function<void(char c)> fct) const
{
    DWord spaces = 0;

    for (DWord index = 0; index < fileHeader.fileSize; index++)
    {
        Byte c = buffer[index];

        if (c == ' ')
        {
            if (++spaces == 127)
            {
                // Expand space compression for a maximum of 127 characters.
                fct(0x09);
                fct(static_cast<Byte>(spaces));
                spaces = 0;
            }
        }
        else
        {
            if (spaces)
            {
                // Expand space compression.
                fct(0x09);
                fct(static_cast<Byte>(spaces));
                spaces = 0;
            }

            if (c > ' ')
            {
                fct(c);
            }
            else if (c == 0x0a)
            {
                // For ASCII LF write ASCII CR indicating end of line
                // in a FLEX text file.
                // If ASCII CR is ignored this works for both Unix/Linux
                // and DOS/Windows text files.
                fct(0x0d);
            }
            else if (c == 0x09)
            {
                // ASCII TAB is converted to 8 spaces.
                fct(0x09);
                fct(8);
            }

            // Other control characters than ASCII TAB or ASCII CR will be
            // ignored.
        }
    } // for
}

// Convert a host operating system text file into a FLEX test file.
// Replace the buffer contents by the converted file contents.
void FlexFileBuffer::ConvertToFlexTextFile()
{
    if (!buffer)
    {
        return;
    }

    DWord new_size = SizeOfConvertedFlexTextFile();
    Byte *new_buffer = new Byte[new_size];
    DWord new_index = 0;

    TraverseForFlexTextFileConversion([&new_buffer, &new_index](char c)
        {
            new_buffer[new_index++] = c;
        });

    buffer.reset(new_buffer);
    fileHeader.fileSize = new_size;
    capacity = new_size;
}

// Estimate the needed buffer size after converting
// a given text file on the host operating system into a FLEX text file.
// Returns the estimated file size in byte.
DWord FlexFileBuffer::SizeOfConvertedFlexTextFile() const
{
    DWord count = 0;

    if (!buffer)
    {
        return count;
    }

    TraverseForFlexTextFileConversion([&count](char)
        {
            count++;
        });

    return count;
}

// Evaluate if the given file is a text file on the host operating system.
bool FlexFileBuffer::IsTextFile() const
{
    for (DWord i = 0; i < fileHeader.fileSize; i++)
    {
        Byte c = buffer[i];

        // Allowed characters of a host operating system text file are:
        // ASCII LF, ASCII CR, ASCII TAB and any character >= ASCII Space
        if (c >= ' ' || c == 0x0a || c == 0x0d || c == 0x09)
        {
            continue;
        }

        return false;
    }

    return true;
}

// Evaluate if the given file is a FLEX text file.
bool FlexFileBuffer::IsFlexTextFile() const
{
    for (DWord i = 0; i < fileHeader.fileSize; i++)
    {
        Byte c = buffer[i];

        // Allowed characters of a FLEX text file are:
        // ASCII LF, ASCII CR, ASCII NUL, ASCII CANCEL and
        // any character >= ASCII Space.
        if (c >= ' ' || c == 0x0a || c == 0x0d || c == 0x00 || c == 0x18)
        {
            continue;
        }

        if (c == 0x09)
        {
            // ASCII TAB is followed by one space count byte.
            i++;
            continue;
        }

        return false;
    }

    return true;
}

// Traverse through a given file and call a function
// for each converted character for converting to a ASCII dump file.
void FlexFileBuffer::TraverseForDumpFileConversion(
        DWord bytesPerLine,
        std::function<void(char c)> fct) const
{
    DWord offset;
    DWord index;
    size_t width = 4;

    width = (fileHeader.fileSize > 0xFFFF) ? 5 : width;
    width = (fileHeader.fileSize > 0xFFFFF) ? 6 : width;

    for (offset = 0; offset < fileHeader.fileSize; offset += bytesPerLine)
    {
        // File offset.
        std::stringstream offsetstream;

        offsetstream << std::setw(width) << std::setfill('0') <<
                        std::hex << offset;
        for (DWord i = 0; i < width; i++)
        {
            fct(offsetstream.str()[i]);
        }
        fct(' ');

        // Hex values for each byte.
        for (index = 0;
             index < bytesPerLine && (offset + index < fileHeader.fileSize);
             index++)
        {
            std::stringstream bytestream;

            auto c = buffer[offset + index] & 0xFF;
            bytestream << std::setw(2) << std::setfill('0') <<
                      std::hex << static_cast<Word>(c);
            fct(bytestream.str()[0]);
            fct(bytestream.str()[1]);
            fct(' ');
        }

        for (; index < bytesPerLine; index++)
        {
            // Fill up with spaces.
            fct(' ');
            fct(' ');
            fct(' ');
        }

        fct(' ');
        fct(' ');

        // ASCII values for each byte.
        for (index = 0;
             index < bytesPerLine && (offset + index < fileHeader.fileSize);
             index++)
        {
            char c = buffer[offset + index] & 0xFF;

            if (c < ' ' || c >= '\x7F')
            {
                c = '_';
            }
            fct(c);
        }

#ifdef _WIN32
        fct(0x0d);
#endif
        fct(0x0a);
    }
}

DWord FlexFileBuffer::SizeOfConvertedDumpFile(DWord bytesPerLine) const
{
    DWord count = 0;

    if (!buffer)
    {
        return count;
    }

    TraverseForDumpFileConversion(bytesPerLine, [&count](char)
        {
            count++;
        });

    return count;
}

void FlexFileBuffer::ConvertToDumpFile(DWord bytesPerLine)
{
    if (!buffer)
    {
        return;
    }

    DWord new_size = SizeOfConvertedDumpFile(bytesPerLine);
    Byte *new_buffer = new Byte[new_size];
    DWord new_index = 0;

    TraverseForDumpFileConversion(bytesPerLine,
                                  [&new_buffer, &new_index](char c)
        {
            new_buffer[new_index++] = c;
        });

    buffer.reset(new_buffer);
    fileHeader.fileSize = new_size;
    capacity = new_size;
}

// Estimate if the given file is a FLEX executable file.
// Not implemented yet.
bool FlexFileBuffer::IsFlexExecutableFile() const
{
    return false;
}

bool FlexFileBuffer::WriteToFile(const char *path) const
{
    BFilePtr fp(path, "wb");

    if (fp != nullptr)
    {
        size_t blocks = fwrite(buffer.get(), GetFileSize(), 1, fp);
        return (blocks == 1);
    }

    return false;
}

#ifdef __GNUC__
bool FlexFileBuffer::WriteToFile(int fd) const
{
    ssize_t bytes;

    bytes = write(fd, buffer.get(), GetFileSize());
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
            size_t blocks = fread(buffer.get(), GetFileSize(), 1, fp);

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
                fileHeader.day = static_cast<Word>(lt->tm_mday);
                fileHeader.month = static_cast<Word>(lt->tm_mon + 1);
                fileHeader.year = static_cast<Word>(lt->tm_year + 1900);
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

void FlexFileBuffer::CopyHeaderBigEndianFrom(const tFlexFileHeader &src)
{
    // Directly copy the file header from a source object.
    // For consistency check verify the magic number.
    DWord oldSize = fileHeader.fileSize;

    memcpy(&fileHeader, &src, sizeof(tFlexFileHeader));
    fileHeader.fileSize = fromBigEndian<DWord>(src.fileSize);
    fileHeader.attributes = fromBigEndian<Word>(src.attributes);
    fileHeader.sectorMap = fromBigEndian<Word>(src.sectorMap);
    fileHeader.day = fromBigEndian<Word>(src.day);
    fileHeader.month = fromBigEndian<Word>(src.month);
    fileHeader.year = fromBigEndian<Word>(src.year);

    for (unsigned index = 0; index < sizeof(fileHeader.magicNumber); ++index)
    {
        if (fileHeader.magicNumber[index] != flexFileHeaderMagicNumber[index])
        {
            std::stringstream stream;

            for (unsigned i = 0; i < sizeof(fileHeader.magicNumber); )
            {
                stream << std::hex <<
                    (0xff & (unsigned)fileHeader.magicNumber[i++]);
            }
            throw FlexException(FERR_INVALID_MAGIC_NUMBER,
                                stream.str().c_str());
        }
    }

    DWord newSize = fileHeader.fileSize;
    fileHeader.fileSize = oldSize;
    Realloc(newSize);
}

bool FlexFileBuffer::CopyFrom(const Byte *from, DWord aSize,
                              DWord offset /* = 0 */)
{
    if (offset + aSize > fileHeader.fileSize)
    {
        return false;
    }

    memcpy(&buffer[offset], from, aSize);
    return true;
}

bool FlexFileBuffer::CopyTo(Byte *to, DWord aSize,
                            DWord offset /* = 0 */,
                            int stuffByte /* = -1 */) const
{
    if (offset + aSize > fileHeader.fileSize)
    {
        if (stuffByte < 0 || offset >= fileHeader.fileSize)
        {
            return false;
        }
        else
        {
            memset(to, stuffByte, aSize);
            memcpy(to, &buffer[offset], fileHeader.fileSize - offset);
            return true;
        }
    }

    memcpy(to, &buffer[offset], aSize);
    return true;
}

void FlexFileBuffer::FillWith(const Byte pattern /* = 0 */)
{
    for (DWord i = 0; i < GetFileSize(); i++)
    {
        buffer[i] = pattern;
    }
}

tFlexFileHeader FlexFileBuffer::GetHeaderBigEndian() const
{
    tFlexFileHeader result;

    memcpy(&result, &fileHeader, sizeof(result));
    result.fileSize = toBigEndian<DWord>(fileHeader.fileSize);
    result.attributes = toBigEndian<Word>(fileHeader.attributes);
    result.sectorMap = toBigEndian<Word>(fileHeader.sectorMap);
    result.day = toBigEndian<Word>(fileHeader.day);
    result.month = toBigEndian<Word>(fileHeader.month);
    result.year = toBigEndian<Word>(fileHeader.year);

    return result;
}

void FlexFileBuffer::SetDate(const BDate &new_date)
{
    fileHeader.day = static_cast<Word>(new_date.GetDay());
    fileHeader.month = static_cast<Word>(new_date.GetMonth());
    fileHeader.year = static_cast<Word>(new_date.GetYear());
}

void FlexFileBuffer::SetDate(int day, int month, int year)
{
    fileHeader.day = static_cast<Word>(day);
    fileHeader.month = static_cast<Word>(month);
    fileHeader.year = static_cast<Word>(year);
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

