/*
    ffilebuf.cpp


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

#include "misc1.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "bdate.h"
#include "ffilebuf.h"
#include "bfileptr.h"
#include "flexerr.h"
#include "fdirent.h"
#include "filecntb.h"
#include <algorithm>


// The format of a FLEX text file is described in the
// FLEX Advanced Programmer's Guide in Chapter
// DESCRIPTION OF A TEXT FILE.

FlexFileBuffer::FlexFileBuffer()
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

FlexFileBuffer::FlexFileBuffer(FlexFileBuffer &&src) noexcept
    : fileHeader(src.fileHeader)
    , capacity(src.capacity)
    , buffer(std::move(src.buffer))
{
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

FlexFileBuffer &FlexFileBuffer::operator=(FlexFileBuffer &&src) noexcept
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
        memcpy(&fileHeader, &src.fileHeader, sizeof(fileHeader));
        capacity = 0;
    }
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
        const std::function<void(Byte c)>& fct) const
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
            Byte spaces = 0;

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
        else if (c == 0x1a)
        {
            break; // ASCII SUB is end of file marker
        }

        // Other control characters than ASCII TAB, ASCII CR, ASCII SUB
        // are ignored.
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

    TraverseForTextFileConversion([&count](Byte)
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

    TraverseForTextFileConversion([&new_buffer, &new_index](Byte c)
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
        const std::function<void(Byte c)>& fct) const
{
    Byte spaces = 0;

    for (DWord index = 0; index < fileHeader.fileSize; index++)
    {
        Byte c = buffer[index];

        if (c == ' ')
        {
            if (++spaces == 127)
            {
                // Expand space compression for a maximum of 127 characters.
                fct(0x09);
                fct(spaces);
                spaces = 0;
            }
        }
        else
        {
            if (spaces)
            {
                if (spaces == 1)
                {
                    fct(' ');
                }
                else
                {
                    // Expand space compression.
                    fct(0x09);
                    fct(spaces);
                }
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

    TraverseForFlexTextFileConversion([&new_buffer, &new_index](Byte c)
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

    TraverseForFlexTextFileConversion([&count](Byte)
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
        // ASCII LF, ASCII CR, ASCII NUL, ASCII CANCEL, ASCII FF, ASCII SUB and
        // any character >= ASCII Space.
        if (c >= ' ' || c == 0x0a || c == 0x0d || c == 0x00 || c == 0x18 ||
            c == 0x0c || c == 0x1a)
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
        const std::function<void(Byte c)>& fct) const
{
    DWord offset;
    DWord index;
    int width = 4;

    width = (fileHeader.fileSize > 0xFFFF) ? 5 : width;
    width = (fileHeader.fileSize > 0xFFFFF) ? 6 : width;

    for (offset = 0; offset < fileHeader.fileSize; offset += bytesPerLine)
    {
        // File offset.
        std::stringstream offsetstream;

        offsetstream << std::setw(width) << std::setfill('0') <<
                        std::hex << offset;
        for (int i = 0; i < width; i++)
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
            auto c = buffer[offset + index];

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

    TraverseForDumpFileConversion(bytesPerLine, [&count](Byte)
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
                                  [&new_buffer, &new_index](Byte c)
        {
            new_buffer[new_index++] = c;
        });

    buffer.reset(new_buffer);
    fileHeader.fileSize = new_size;
    capacity = new_size;
}

// Estimate if the given file is a FLEX executable file.
// Implementation may change in future (unfinished).
// NOLINTNEXTLINE(readability-convert-member-functions-to-static).
bool FlexFileBuffer::IsFlexExecutableFile() const
{
    return false;
}

bool FlexFileBuffer::WriteToFile(const char *path) const
{
    BFilePtr fp(path, "wb");

    if (fp != nullptr)
    {
        if (GetFileSize() != 0)
        {
            size_t blocks = fwrite(buffer.get(), GetFileSize(), 1, fp);
            return (blocks == 1);
        }
        return true;
    }

    return false;
}

bool FlexFileBuffer::ReadFromFile(const char *path)
{
    struct stat sbuf;

    if (!stat(path, &sbuf) && S_ISREG(sbuf.st_mode) && sbuf.st_size >= 0)
    {
        BFilePtr fp(path, "rb");

        Realloc(sbuf.st_size);

        if (fp != nullptr)
        {
            size_t blocks = 0;

            if (GetFileSize() > 0)
            {
                blocks = fread(buffer.get(), GetFileSize(), 1, fp);
            }

            if (blocks == 1 || GetFileSize() == 0)
            {
                auto fullPath = toAbsolutePath(path);
                auto directory = getParentPath(fullPath);

                SetAttributes(0);
                SetSectorMap(0);

                if(access(directory.c_str(), W_OK))
                {
                    // CDFS-Support: look for file name in file 'random'
                    if (isListedInFileRandom(directory.c_str(),
                                             getFileName(path).c_str()))
                    {
                        SetSectorMap(IS_RANDOM_FILE);
                    }
                }
                else if (hasRandomFileAttribute(directory.c_str(),
                                                getFileName(path).c_str()))
                {
                    SetSectorMap(IS_RANDOM_FILE);
                }

                if(access(path, W_OK))
                {
                    SetAttributes(FLX_READONLY);
                }

                SetAdjustedFilename(getFileName(path));
                struct tm *lt = localtime(&(sbuf.st_mtime));
                SetDateTime(
                        BDate(lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900),
                        BTime(lt->tm_hour, lt->tm_min));

                return true;
            }
        }
    }

    return false;
}

// Adjust the file name so that it conforms to 8.3
// Copy file name into file header struct.
void FlexFileBuffer::SetAdjustedFilename(const std::string &fileName)
{
    auto pos = fileName.find('.');
    auto baseNameSize = (pos == std::string::npos) ?
        FLEX_BASEFILENAME_LENGTH : std::min(pos, FLEX_BASEFILENAME_LENGTH);
    auto adjustedFileName = fileName.substr(0U, baseNameSize);
    if (pos != std::string::npos)
    {
        auto extension = fileName.substr(pos + 1U, FLEX_FILEEXT_LENGTH);
        if (!extension.empty())
        {
            adjustedFileName.append(".").append(extension);
        }
    }
    strupper(adjustedFileName);
    strncpy(fileHeader.fileName, adjustedFileName.c_str(),
            sizeof(fileHeader.fileName) - 1U);
    fileHeader.fileName[sizeof(fileHeader.fileName) - 1U] = '\0';
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
                    (0xff & static_cast<unsigned>(fileHeader.magicNumber[i++]));
            }
            throw FlexException(FERR_INVALID_MAGIC_NUMBER,
                                stream.str());
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

    if (buffer != nullptr)
    {
        memcpy(&buffer[offset], from, aSize);
    }
    return true;
}

bool FlexFileBuffer::CopyTo(Byte *to, DWord aSize,
                            DWord offset /* = 0 */,
                            int stuffByte /* = -1 */) const
{
    if (to == nullptr)
    {
            throw FlexException(FERR_WRONG_PARAMETER);
    }

    if (offset + aSize > fileHeader.fileSize)
    {
        if (stuffByte < 0 || offset >= fileHeader.fileSize)
        {
            return false;
        }

        memset(to, stuffByte, aSize);
        if (buffer != nullptr)
        {
            memcpy(to, &buffer[offset], fileHeader.fileSize - offset);
        }

        return true;
    }
    if (buffer != nullptr)
    {
        memcpy(to, &buffer[offset], aSize);
    }
    return true;
}

void FlexFileBuffer::FillWith(Byte pattern /* = 0 */)
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

void FlexFileBuffer::SetDateTime(const BDate &new_date, const BTime &new_time)
{
    fileHeader.day = static_cast<Word>(new_date.GetDay());
    fileHeader.month = static_cast<Word>(new_date.GetMonth());
    fileHeader.year = static_cast<Word>(new_date.GetYear());
    fileHeader.hour = static_cast<Word>(new_time.GetHour());
    fileHeader.minute = static_cast<Word>(new_time.GetMinute());
}

BDate FlexFileBuffer::GetDate() const
{
    return {fileHeader.day, fileHeader.month, fileHeader.year};
}

BTime FlexFileBuffer::GetTime() const
{
    return {fileHeader.hour, fileHeader.minute, 0U};
}

