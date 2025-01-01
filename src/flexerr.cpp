/*
    flexerr.cpp


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

#include <exception>
#include <sstream>
#include "flexerr.h"
#include "cvtwchar.h"
#include <array>
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"


const char *FlexException::what() const noexcept
{
    return errorString.c_str();
}

FlexException::FlexException(const FlexException &src) noexcept:
    errorCode(src.errorCode), errorString(src.errorString)
{
}

FlexException &FlexException::operator= (const FlexException &src) noexcept
{
    if (this != &src)
    {
        errorCode = src.errorCode;
        errorString = src.errorString;
    }

    return *this;
}

FlexException::FlexException() noexcept : errorCode(FERR_FLEX_EXCEPTION)
{
}

FlexException::FlexException(int ec) noexcept
    : errorCode(ec)
    , errorString(errString[ec])
{
}

FlexException::FlexException(int ec, int ip1) noexcept
    : errorCode(ec)
{
    errorString = fmt::format(errString[ec], ip1);
}

FlexException::FlexException(int ec, const std::string &sp1) noexcept
    : errorCode(ec)
{
    errorString = fmt::format(errString[ec], sp1);
}

FlexException::FlexException(int ec, const std::string &sp1,
                             const std::string &sp2) noexcept
    : errorCode(ec)
{
    errorString = fmt::format(errString[ec], sp1, sp2);
}

FlexException::FlexException(int ec, int ip1, const std::string &sp1) noexcept
    : errorCode(ec)
{
    errorString = fmt::format(errString[ec], ip1, sp1);
}

FlexException::FlexException(int ec, int ip1, int ip2, const std::string &sp1)
    noexcept
    : errorCode(ec)
{
    errorString = fmt::format(errString[ec], ip1, ip2, sp1);
}

FlexException::FlexException(int ec, int ip1, const std::string &sp1,
        const std::string &sp2) noexcept
    : errorCode(ec)
{
    errorString = fmt::format(errString[ec], ip1, sp1, sp2);
}

FlexException::FlexException(int ec, const std::string &sp1,
        const std::string &sp2, const std::string &sp3) noexcept
    : errorCode(ec)
{
    errorString = fmt::format(errString[ec], sp1, sp2, sp3);
}

#ifdef _WIN32
FlexException::FlexException(unsigned long lastError, const std::string &sp1)
    noexcept
{
    LPWSTR lpMsgBuf = nullptr;

    if (!FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            nullptr, lastError, 0, (LPWSTR)&lpMsgBuf, 0, nullptr))
    {
        errorCode = FERR_UNSPEC_WINDOWS_ERROR;
        errorString = fmt::format(errString[errorCode], sp1);
        return;
    }

    errorCode = FERR_WINDOWS_ERROR;

    errorString = ConvertToUtf8String(lpMsgBuf);
    errorString += sp1;

    LocalFree(lpMsgBuf);
}
#endif

std::array<const char *, FERR_COUNT> FlexException::errString
{
    "No Error",
    "Unable to open {0}",
    "{0} is no FLEX disk image file",
    "No disk image file opened",
    "No file opened",
    "Unable to format {0}",
    "Invalid disk image format #{0}",
    "Error reading from {0}",
    "Error writing to {0}",
    "Directory already opened",
    "No directory opened",
    "File already opened",
    "No free file handle available",
    "File {0} already exists",
    "Invalid file handle #{0}",
    "Invalid open mode \"{0}\"",
    "Directory full",
    "Error reading track-sector {0} in {1}",
    "Error writing track-sector {0} in {1}",
    "No file \"{0}\" (disk image {1})",
    "Record map of {0} is full (disk image {1})",
    "Disk image {0} full when writing {1}",
    "Unable to create {0}",
    "Unable to rename {0} (disk image {1})",
    "Unable to remove {0} (disk image {1})",
    "Error reading disk space (disk image {0})",
    "Unable to copy {0} on itself",
    "Wrong parameter",
    "Error creating process ({0} {1})",
    "Error reading FLEX binary format",
    "Error creating temporary file {0}",
    "Disk image {0} is read-only",
    "An unspecified Windows error occured (#{1})",
    "",
    "{0} is an invalid NULL pointer",
    "",
    "Unsupported GUI type (#{0})",
    "Invalid magic number 0x{0}",
    "In file {2} line {0} is invalid: \"{1}\"",
    "Invalid usage of disk image iterator",
    "File {0} has unexpected sector count {1}",
    "Disk image {0} is unformatted or has unknown format",
    "Unexpected side number {0}",
    "Empty files can not be copied. File {0}",
    "Error {0} on system call '{1}'",
    "Disk image {0} has invalid or unsupported JVC header",
    "Bad optional access",
    "Wildcard '{0}' not supported",
    "{0} is no MDCR formatted file",
    "Unable to remove {0}",
    "Invalid terminal type {0}",
};

