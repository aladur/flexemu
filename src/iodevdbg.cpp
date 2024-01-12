/*
    iodevdbg.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2023  W. Schwotzer

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
#include <iomanip>


IoDeviceDebug::IoDeviceDebug(IoDevice &x_device,
                             const std::string &x_logFilePath) :
      device(x_device)
    , logFilePath(x_logFilePath)
{
    std::ofstream fstream(logFilePath,
                          std::ios_base::out | std::ios_base::trunc);

    if (fstream.is_open())
    {
        // Just truncate and close file.
        fstream.close();
    }
}

IoDeviceDebug::IoDeviceDebug(IoDeviceDebug &&src) noexcept :
      device(src.device)
    , logFilePath(std::move(src.logFilePath))
{
}

Byte IoDeviceDebug::readIo(Word offset)
{
    std::ofstream fstream(logFilePath, std::ios_base::out | std::ios_base::app);

    Byte result = device.readIo(offset);

    if (fstream.is_open())
    {
        fstream << "mode=read"
                << "  offset=" << std::setw(2) << offset
                << " result=" << tohexstr(result)
                << " device=" << std::string(device.getName())
                << std::endl;

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
        fstream << "mode=write"
                << " offset=" << std::setw(2) << offset
                << " value=" << tohexstr(value)
                << "  device=" << std::string(device.getName())
                << std::endl;

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
                << std::endl;

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

