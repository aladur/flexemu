/*
    iodevdbg.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2024  W. Schwotzer

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

#include "iodevdbg.h"
#include "misc1.h"
#include <fstream>
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"


IoDeviceDebug::IoDeviceDebug(IoDevice &p_device, std::string p_logFilePath)
    : device(p_device)
    , logFilePath(std::move(p_logFilePath))
{
    std::ofstream fstream(logFilePath,
                          std::ios_base::out | std::ios_base::trunc);

    if (fstream.is_open())
    {
        // Just truncate and close file.
        fstream.close();
    }
}

IoDeviceDebug::IoDeviceDebug(IoDeviceDebug &&src) noexcept
    : device(src.device)
    , logFilePath(std::move(src.logFilePath))
{
}

Byte IoDeviceDebug::readIo(Word offset)
{
    std::ofstream fstream(logFilePath, std::ios_base::out | std::ios_base::app);

    Byte result = device.readIo(offset);

    if (fstream.is_open())
    {
        fstream << fmt::format(
                "mode=read  offset={:2} result={:02X} device={}\n",
                offset, static_cast<Word>(result), device.getName());
        fstream.close();
    }

    return result;
}

void IoDeviceDebug::writeIo(Word offset, Byte value)
{
    std::ofstream fstream(logFilePath, std::ios_base::out | std::ios_base::app);

    device.writeIo(offset, value);

    if (fstream.is_open())
    {
        fstream << fmt::format(
                "mode=write offset={:2} value={:02X}  device={}\n",
                offset, static_cast<Word>(value), device.getName());
        fstream.close();
    }
}

void IoDeviceDebug::resetIo()
{
    std::ofstream fstream(logFilePath, std::ios_base::out | std::ios_base::app);

    device.resetIo();

    if (fstream.is_open())
    {

        fstream << "mode=reset device=" << std::string(device.getName())
                << '\n';

        fstream.close();
    }
}

const char *IoDeviceDebug::getName()
{
    return device.getName();
}

Word IoDeviceDebug::sizeOfIo()
{
    return device.sizeOfIo();
}

