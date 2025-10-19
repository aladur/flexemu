/*
    iodevdbg.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2025  W. Schwotzer

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
#include <string>

class IoDeviceDebug : public IoDevice
{
public:

    IoDeviceDebug() = delete;
    ~IoDeviceDebug() override = default;
    IoDeviceDebug(IoDevice &p_device, std::string p_logFilePath);
    IoDeviceDebug(const IoDeviceDebug &src) = default;
    IoDeviceDebug(IoDeviceDebug &&src) noexcept;
    // Avoid using copy or move assignment because of device reference
    // which can not be reassigned.
    IoDeviceDebug& operator=(const IoDeviceDebug &src) = delete;
    IoDeviceDebug& operator=(IoDeviceDebug &&src) = delete;

    // IoDevice interface
    Byte readIo(Word offset) override;
    void writeIo(Word offset, Byte value) override;
    void resetIo() override;
    const char *getName() override;
    const char *getDescription() override;
    const char *getClassName() override;
    const char *getClassDescription() override;
    const char *getVendor() override;
    Word sizeOfIo() override;

private:
    // Intentionally use a reference.
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    IoDevice &device;
    std::string logFilePath;
};

#endif

