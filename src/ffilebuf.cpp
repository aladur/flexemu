/*
    ffilebuf.cpp


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

#include "config.h"
#include "typedefs.h"
#include "misc1.h"
#include "asciictl.h"
#include "efiletim.h"
#include "filecnts.h"
#include "bdate.h"
#include "ffilebuf.h"
#include "flexerr.h"
#include "fdirent.h"
#include "filecntb.h"
#include "rndcheck.h"
#include <sys/stat.h>
#ifdef _WIN32
#include <sys/utime.h>
#else
#include <utime.h>
#endif
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ios>
#include <ctime>
#include <cstring>
#include <utility>
#include <functional>
#include <optional>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <filesystem>
#ifdef _WIN32
#include "windefs.h"
#endif

namespace fs = std::filesystem;

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
    , buffer(std::move(src.buffer))
{
    memset(&src.fileHeader, 0, sizeof(src.fileHeader));
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
        memcpy(&fileHeader, &src.fileHeader, sizeof(fileHeader));
        memset(&src.fileHeader, 0, sizeof(src.fileHeader));
    }

    return *this;
}

void FlexFileBuffer::copyFrom(const FlexFileBuffer &src)
{
    if (!src.buffer.empty())
    {
        buffer.resize(src.fileHeader.fileSize);
        std::copy(src.buffer.cbegin(),
                  src.buffer.cbegin() + src.fileHeader.fileSize,
                  buffer.begin());
    }
    memcpy(&fileHeader, &src.fileHeader, sizeof(fileHeader));
}

const Byte *FlexFileBuffer::GetBuffer(DWord offset /* = 0*/,
                                      DWord bytes /* = 1 */) const
{
    if (offset + bytes > fileHeader.fileSize)
    {
        return nullptr;
    }

    return buffer.data() + offset;
}

void FlexFileBuffer::SetFilename(const std::string &fileName)
{
    strncpy(fileHeader.fileName, fileName.c_str(), FLEX_FILENAME_LENGTH - 1);
    fileHeader.fileName[sizeof(fileHeader.fileName) - 1] = '\0';
}

// Reallocate the buffer with a different size.
// Buffer will be initialized to zero or
// optionally with a copy of the contents of the old buffer.
void FlexFileBuffer::Realloc(DWord newSize,
                             bool restoreContents /* = false*/)
{
    if (newSize == 0U)
    {
        return;
    }

    if (buffer.size() > fileHeader.fileSize)
    {
        std::fill(buffer.begin() + fileHeader.fileSize, buffer.end(), '\0');
    }
    buffer.resize(newSize, '\0');
    if (!restoreContents)
    {
        std::fill(buffer.begin(), buffer.begin() + fileHeader.fileSize, '\0');
    }
    fileHeader.fileSize = newSize;
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
        else if (c == CR)
        {
            // Convert ASCII CR, the FLEX text file end of line character
            // into a new line (depending on the operating system).
#ifdef _WIN32
            fct(CR);
#endif
            fct(LF);
        }
        else if (c == HT)
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
        else if (c == SUB)
        {
            break; // ASCII SUB is end of file marker
        }

        // Other control characters than ASCII TAB, ASCII CR, ASCII SUB
        // are ignored.
    }
}

// Estimate the needed buffer size after converting
// a given FLEX text file into a text file on the host operating
// system.
// Returns the estimated file size in byte.
DWord FlexFileBuffer::SizeOfConvertedTextFile() const
{
    DWord count = 0;

    if (buffer.empty())
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
    if (buffer.empty() || fileHeader.fileSize == 0)
    {
        return;
    }

    DWord new_size = SizeOfConvertedTextFile();
    std::vector<Byte> new_buffer(new_size);
    DWord new_index = 0;

    TraverseForTextFileConversion([&new_buffer, &new_index](Byte c)
        {
            new_buffer[new_index++] = c;
        });

    buffer = std::move(new_buffer);
    fileHeader.fileSize = new_size;
}

// Traverse through a given host operating system text file and call a function
// for each converted character for converting to a FLEX text file.
void FlexFileBuffer::TraverseForFlexTextFileConversion(
        const std::function<void(Byte c)>& fct) const
{
    Byte spaces = 0;
    auto process_spaces = [&](){
        if (spaces)
        {
            if (spaces == 1)
            {
                fct(' ');
            }
            else
            {
                // Do space compression.
                fct(HT);
                fct(spaces);
            }
            spaces = 0;
        }
    };

    for (DWord index = 0; index < fileHeader.fileSize; index++)
    {
        Byte c = buffer[index];

        if (c == ' ')
        {
            if (++spaces == 127)
            {
                // Do space compression for a maximum of 127 characters.
                process_spaces();
            }
        }
        else
        {
            process_spaces();
            if (c > ' ')
            {
                fct(c);
            }
            else if (c == LF)
            {
                // For ASCII LF write ASCII CR indicating end of line
                // in a FLEX text file.
                // If ASCII CR is ignored this works for both Unix/Linux
                // and DOS/Windows text files.
                fct(CR);
            }
            else if (c == HT)
            {
                // ASCII TAB is converted to 8 spaces.
                fct(HT);
                fct(8);
            }

            // Other control characters than ASCII TAB or ASCII CR will be
            // ignored.
        }
    }

    // Process remaining spaces if file does not end with new line.
    process_spaces();
}

// Convert a host operating system text file into a FLEX test file.
// Replace the buffer contents by the converted file contents.
void FlexFileBuffer::ConvertToFlexTextFile()
{
    if (buffer.empty())
    {
        return;
    }

    DWord new_size = SizeOfConvertedFlexTextFile();
    std::vector<Byte> new_buffer(new_size);
    DWord new_index = 0;

    TraverseForFlexTextFileConversion([&new_buffer, &new_index](Byte c)
        {
            new_buffer[new_index++] = c;
        });

    buffer = std::move(new_buffer);
    fileHeader.fileSize = new_size;
}

// Estimate the needed buffer size after converting
// a given text file on the host operating system into a FLEX text file.
// Returns the estimated file size in byte.
DWord FlexFileBuffer::SizeOfConvertedFlexTextFile() const
{
    DWord count = 0;

    if (buffer.empty())
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
        if (c >= ' ' || c == LF || c == CR || c == HT)
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
        if (c >= ' ' || c == LF || c == CR || c == NUL || c == CAN ||
            c == FF || c == SUB)
        {
            continue;
        }

        if (c == HT)
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

        const auto offset_str = fmt::format("{0:0{1}X}", offset, width);
        for (int i = 0; i < width; i++)
        {
            fct(offset_str[i]);
        }
        fct(' ');

        // Hex values for each byte.
        for (index = 0;
             index < bytesPerLine && (offset + index < fileHeader.fileSize);
             index++)
        {
            auto c = buffer[offset + index] & 0xFFU;
            const auto byte_str = fmt::format("{:02X}", static_cast<Word>(c));
            fct(byte_str[0]);
            fct(byte_str[1]);
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

            if (c < ' ' || c >= DEL)
            {
                c = '_';
            }
            fct(c);
        }

#ifdef _WIN32
        fct(CR);
#endif
        fct(LF);
    }
}

DWord FlexFileBuffer::SizeOfConvertedDumpFile(DWord bytesPerLine) const
{
    DWord count = 0;

    if (buffer.empty())
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
    if (buffer.empty())
    {
        return;
    }

    DWord new_size = SizeOfConvertedDumpFile(bytesPerLine);
    std::vector<Byte> new_buffer(new_size);
    DWord new_index = 0;

    TraverseForDumpFileConversion(bytesPerLine,
                                  [&new_buffer, &new_index](Byte c)
        {
            new_buffer[new_index++] = c;
        });

    buffer = std::move(new_buffer);
    fileHeader.fileSize = new_size;
}

// Estimate if the given file is a FLEX executable file.
bool FlexFileBuffer::IsFlexExecutableFile() const
{
    ReadCmdState state = ReadCmdState::GetType;
    int count = 0;

    for (DWord i = 0; i < fileHeader.fileSize; ++i)
    {
        Byte byte = buffer[i];

        switch (state)
        {
            case ReadCmdState::GetType:
                switch (byte)
                {
                    case '\x02':
                        count = 2;
                        state = ReadCmdState::GetDataAddress;
                        continue;

                    case '\x16':
                        count = 2;
                        state = ReadCmdState::GetStartAddress;
                        continue;

                    case '\x00':
                        state = ReadCmdState::GetNUL;
                        continue;

                    default:
                        return false;
                }

            case ReadCmdState::GetDataAddress:
                state = (--count == 0) ? ReadCmdState::GetCount : state;
                continue;

            case ReadCmdState::GetCount:
                count = byte;
                state = ReadCmdState::GetData;
                continue;

            case ReadCmdState::GetData:
            case ReadCmdState::GetStartAddress:
                state = (--count == 0) ? ReadCmdState::GetType : state;
                continue;

            case ReadCmdState::GetNUL:
                if (byte != '\0')
                {
                    return false;
                }
                continue;
        }
    }

    return true;
}

bool FlexFileBuffer::WriteToFile(const fs::path &path,
        FileTimeAccess fileTimeAccess, bool doRandomCheck) const
{
    auto mode = std::ios::out | std::ios::binary | std::ios::trunc;
    std::ofstream ostream(path, mode);
    bool result = false;

    if (!ostream.is_open() || GetFileSize() == 0)
    {
        return result;
    }

    ostream.write(reinterpret_cast<const char *>(buffer.data()),
                  GetFileSize());
    result = ostream.good();
    ostream.close();

    const auto directory = path.parent_path();
    if (doRandomCheck)
    {
        RandomFileCheck randomFileCheck(directory);

        randomFileCheck.CheckAllFilesAttributeAndUpdate();
        if (IsRandom())
        {
            randomFileCheck.AddToRandomList(GetFilename());
        }
    }

    const bool setFileTime =
        (fileTimeAccess & FileTimeAccess::Set) == FileTimeAccess::Set;

    if (result && fs::exists(path))
    {
        struct utimbuf timebuf{};
        struct tm file_time{};
        struct stat sbuf{};

#ifdef _WIN32
        _wstat(path.wstring().c_str(), &sbuf);
#else
        stat(path.u8string().c_str(), &sbuf);
#endif
        timebuf.actime = sbuf.st_atime;
        file_time.tm_sec = 0;
        file_time.tm_min = setFileTime ? fileHeader.minute : 0;
        file_time.tm_hour = setFileTime ? fileHeader.hour : 0;
        file_time.tm_mon = fileHeader.month - 1;
        file_time.tm_mday = fileHeader.day;
        file_time.tm_year = fileHeader.year - 1900;
        file_time.tm_isdst = -1;
        timebuf.modtime = mktime(&file_time);
        return (timebuf.modtime >= 0 &&
#ifdef _WIN32
                _wutime(path.wstring().c_str(), &timebuf) == 0);
#else
                utime(path.u8string().c_str(), &timebuf) == 0);
#endif
    }

    return result;
}

bool FlexFileBuffer::ReadFromFile(const fs::path &path,
        FileTimeAccess fileTimeAccess, bool doRandomCheck)
{
    const auto absPath = fs::absolute(path);
    const auto status = fs::status(absPath);
    if (exists(status) && fs::is_regular_file(status))
    {
        std::ifstream istream(absPath, std::ios::in | std::ios::binary);

        Realloc(static_cast<DWord>(fs::file_size(absPath)));

        if (istream.is_open())
        {
            if (GetFileSize() > 0)
            {
                istream.read(reinterpret_cast<char *>(buffer.data()),
                             GetFileSize());
            }

            if (GetFileSize() == 0 || istream.good())
            {
                const auto directory = absPath.parent_path();
                const auto filename = absPath.filename().u8string();
                struct stat sbuf{};

                SetAttributes(0);
                SetSectorMap(0);

                if (doRandomCheck)
                {
                    RandomFileCheck randomFileCheck(directory);

                    randomFileCheck.CheckAllFilesAttributeAndUpdate();
                    if (randomFileCheck.CheckForRandom(filename))
                    {
                        SetSectorMap(IS_RANDOM_FILE);
                    }
                }

#ifdef _WIN32
                if(_waccess(absPath.wstring().c_str(), W_OK))
#else
                if(access(absPath.u8string().c_str(), W_OK))
#endif
                {
                    SetAttributes(FLX_READONLY);
                }

                SetAdjustedFilename(filename);

#ifdef _WIN32
                _wstat(absPath.wstring().c_str(), &sbuf);
#else
                stat(absPath.u8string().c_str(), &sbuf);
#endif
                struct tm *lt = localtime(&sbuf.st_mtime);
                const bool getFileTime =
                (fileTimeAccess & FileTimeAccess::Get) == FileTimeAccess::Get;
                if (!getFileTime)
                {
                    lt->tm_hour = 0U;
                    lt->tm_min = 0U;
                }
                SetDateTime({lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900},
                            {lt->tm_hour, lt->tm_min});

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
    flx::strupper(adjustedFileName);
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
    fileHeader.fileSize = flx::fromBigEndian<DWord>(src.fileSize);
    fileHeader.attributes = flx::fromBigEndian<Word>(src.attributes);
    fileHeader.sectorMap = flx::fromBigEndian<Word>(src.sectorMap);
    fileHeader.day = flx::fromBigEndian<Word>(src.day);
    fileHeader.month = flx::fromBigEndian<Word>(src.month);
    fileHeader.year = flx::fromBigEndian<Word>(src.year);
    fileHeader.hour = flx::fromBigEndian<Word>(src.hour);
    fileHeader.minute = flx::fromBigEndian<Word>(src.minute);

    for (unsigned index = 0; index < sizeof(fileHeader.magicNumber); ++index)
    {
        if (fileHeader.magicNumber[index] != flexFileHeaderMagicNumber[index])
        {
            std::stringstream stream;

            for (const auto byte : fileHeader.magicNumber)
            {
                stream << std::hex << static_cast<Word>(byte);
            }
            throw FlexException(FERR_INVALID_MAGIC_NUMBER,
                                stream.str());
        }
    }

    DWord newSize = fileHeader.fileSize;
    fileHeader.fileSize = oldSize;
    Realloc(newSize);
}

bool FlexFileBuffer::CopyFrom(const Byte *source, DWord size,
                              DWord offset /* = 0 */)
{
    if (source == nullptr)
    {
            throw FlexException(FERR_WRONG_PARAMETER);
    }

    if (offset + size > fileHeader.fileSize)
    {
        return false;
    }

    if (!buffer.empty())
    {
        memcpy(&buffer[offset], source, size);
    }
    return true;
}

bool FlexFileBuffer::CopyTo(Byte *target, DWord size,
                            DWord offset /* = 0 */,
                            std::optional<Byte> stuffByte
                            /* = std::nullopt */) const
{
    if (target == nullptr)
    {
            throw FlexException(FERR_WRONG_PARAMETER);
    }

    if (offset + size > fileHeader.fileSize)
    {
        if (!stuffByte.has_value() || offset >= fileHeader.fileSize)
        {
            return false;
        }

        memset(target, stuffByte.value(), size);
        if (!buffer.empty())
        {
            memcpy(target, &buffer[offset], fileHeader.fileSize - offset);
        }

        return true;
    }
    if (!buffer.empty())
    {
        memcpy(target, &buffer[offset], size);
    }
    return true;
}

void FlexFileBuffer::FillWith(Byte pattern /* = '\0' */)
{
    std::fill(buffer.begin(), buffer.begin() + GetFileSize(), pattern);
}

tFlexFileHeader FlexFileBuffer::GetHeaderBigEndian() const
{
    tFlexFileHeader result{};

    memcpy(&result, &fileHeader, sizeof(result));
    result.fileSize = flx::toBigEndian<DWord>(fileHeader.fileSize);
    result.attributes = flx::toBigEndian<Word>(fileHeader.attributes);
    result.sectorMap = flx::toBigEndian<Word>(fileHeader.sectorMap);
    result.day = flx::toBigEndian<Word>(fileHeader.day);
    result.month = flx::toBigEndian<Word>(fileHeader.month);
    result.year = flx::toBigEndian<Word>(fileHeader.year);
    result.hour = flx::toBigEndian<Word>(fileHeader.hour);
    result.minute = flx::fromBigEndian<Word>(fileHeader.minute);

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

FlexDirEntry FlexFileBuffer::GetDirEntry() const
{
    FlexDirEntry dirEntry;

    if (!buffer.empty())
    {
        dirEntry.SetTotalFileName(fileHeader.fileName);
        dirEntry.SetFileSize(fileHeader.fileSize / DBPS * SECTOR_SIZE);
        dirEntry.SetAttributes(static_cast<Byte>(fileHeader.attributes));
        dirEntry.SetDate(GetDate());
        dirEntry.SetTime(GetTime());
        dirEntry.SetSectorMap(fileHeader.sectorMap);
        // Start and end track/sector is not set in this case.
        dirEntry.ClearEmpty();
    }

    return dirEntry;
}

