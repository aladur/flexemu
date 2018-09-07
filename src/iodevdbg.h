/*
    iodevdbg.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018  W. Schwotzer

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



#ifndef IODEVDBG_INCLUDED
#define IODEVDBG_INCLUDED

#include "iodevice.h"
#include <functional>

class IoDeviceDebug : public IoDevice
{
public:

    IoDeviceDebug() = delete;
    IoDeviceDebug(const IoDeviceDebug &) = delete;
    IoDeviceDebug(IoDevice &x_device, const std::string &x_logFilePath);
    IoDeviceDebug(IoDeviceDebug &&);
    // Avoid using copy or move assignment because of device reference
    // which can not be reassigned.
    IoDeviceDebug& operator=(const IoDeviceDebug &) = delete;
    IoDeviceDebug& operator=(IoDeviceDebug &&) = delete;

    // IoDevice interface
    Byte readIo(Word offset) override;
    void writeIo(Word offset, Byte value) override;
    void resetIo() override;
    const char *getName() override;
    Word sizeOfIo() override;

private:
    IoDevice &device;
    std::string logFilePath;
};

#endif

