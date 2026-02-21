/*
    flexerr.h


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 1998-2026  W. Schwotzer

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

#ifndef FLEXERR_INCLUDED
#define FLEXERR_INCLUDED

#include <cstdint>
#include <exception>
#include <string>
#include <array>
#include <filesystem>

namespace fs = std::filesystem;


enum : uint8_t {
FERR_COUNT = 51,
};

enum : uint8_t {
FERR_NOERROR = 0,
FERR_UNABLE_TO_OPEN = 1,
FERR_IS_NO_FILECONTAINER = 2,
FERR_DUMMY0 = 3,
FERR_DUMMY1 = 4,
FERR_UNABLE_TO_FORMAT = 5,
FERR_INVALID_FORMAT = 6,
FERR_READING_FROM = 7,
FERR_WRITING_TO = 8,
FERR_DUMMY2 = 9,
FERR_DUMMY3 = 10,
FERR_DUMMY4 = 11,
FERR_DUMMY5 = 12,
FERR_FILE_ALREADY_EXISTS = 13,
FERR_DUMMY6 = 14,
FERR_DUMMY7 = 15,
FERR_DIRECTORY_FULL = 16,
FERR_READING_TRKSEC = 17,
FERR_WRITING_TRKSEC = 18,
FERR_NO_FILE_IN_CONTAINER = 19,
FERR_RECORDMAP_FULL = 20,
FERR_DISK_FULL_WRITING = 21,
FERR_UNABLE_TO_CREATE = 22,
FERR_RENAME_FILE = 23,
FERR_REMOVE_FILE = 24,
FERR_READING_DISKSPACE = 25,
FERR_COPY_ON_ITSELF = 26,
FERR_WRONG_PARAMETER = 27,
FERR_CREATE_PROCESS = 28,
FERR_DUMMY8 = 29,
FERR_DUMMY9 = 30,
FERR_CONTAINER_IS_READONLY = 31,
FERR_UNSPEC_WINDOWS_ERROR = 32,
FERR_WINDOWS_ERROR = 33,
FERR_DUMMY10 = 34,
FERR_FLEX_EXCEPTION = 35,
FERR_DUMMY11 = 36,
FERR_INVALID_MAGIC_NUMBER = 37,
FERR_INVALID_LINE_IN_FILE = 38,
FERR_INVALID_ITERATOR_USE = 39,
FERR_FILE_UNEXPECTED_SEC = 40,
FERR_CONTAINER_UNFORMATTED = 41,
FERR_NO_SIDE_NUMBER = 42,
FERR_COPY_EMPTY_FILE = 43,
FERR_ERROR_IN_SYSTEM_CALL = 44,
FERR_INVALID_JVC_HEADER = 45,
FERR_DUMMY13 = 46,
FERR_WILDCARD_NOT_SUPPORTED = 47,
FERR_IS_NO_MDCRFORMAT = 48,
FERR_REMOVE = 49,
FERR_INVALID_TERMINAL_TYPE = 50,
};


class FlexException : public std::exception
{
protected:

    int errorCode;
    std::string errorString;
    static std::array<const char *, FERR_COUNT> errString;

public:

    FlexException() noexcept;
    FlexException(const FlexException &src) noexcept;
    FlexException& operator= (const FlexException &src) noexcept;
    FlexException(FlexException &&src) = delete;
    FlexException& operator= (FlexException &&src) = delete;
    ~FlexException() override = default;

    explicit FlexException(int ec) noexcept;
    FlexException(int ec, int ip1) noexcept;
    FlexException(int ec, const char *cp1) noexcept;
    FlexException(int ec, const std::string &sp1) noexcept;
    FlexException(int ec, const fs::path &pp1) noexcept;
    FlexException(int ec, const std::string &sp1,
        const std::string &sp2) noexcept;
    FlexException(int ec, const std::string &sp1, const fs::path &pp1) noexcept;
    FlexException(int ec, const fs::path &pp1, const std::string &sp1) noexcept;
    FlexException(int ec, int ip1, const std::string &sp1) noexcept;
    FlexException(int ec, int ip1, const fs::path &pp1) noexcept;
    FlexException(int ec, int ip1, int ip2, const std::string &sp1) noexcept;
    FlexException(int ec, int ip1, const std::string &sp1,
                  const fs::path &pp1) noexcept;
    FlexException(int ec, const std::string &sp1, const std::string &sp2,
                  const std::string &sp3) noexcept;
#ifdef _WIN32
    FlexException(unsigned long lastError, const std::string &sp1) noexcept;
#endif

    const char *what() const noexcept override;
    int GetErrorCode() const
    {
        return errorCode;
    };
};

#endif // FLEXERR_INCLUDED
