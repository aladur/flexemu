/*
    fcnffile.h


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

#ifndef FCNFFILE_INCLUDED
#define FCNFFILE_INCLUDED

#include <vector>
#include <string>
#include <set>


struct sIoDeviceMapping
{
    std::string name;
    Word baseAddress;
    int byteSize;
};


class FlexemuConfigFile
{
public:
    FlexemuConfigFile() = delete;
    FlexemuConfigFile(const FlexemuConfigFile &src) = delete;
    FlexemuConfigFile(FlexemuConfigFile &&src) noexcept;
    explicit FlexemuConfigFile(const char *aFileName);
    ~FlexemuConfigFile() = default;

    FlexemuConfigFile &operator=(const FlexemuConfigFile &) = delete;
    FlexemuConfigFile &operator=(FlexemuConfigFile &&src) noexcept;

    std::vector<sIoDeviceMapping> ReadIoDevices();
    std::string GetDebugSupportOption(const std::string &key);
    std::pair<std::string, std::set<std::string> > GetIoDeviceLogging();
    int GetSerparAddress(const std::string &monitorFilePath);

    static const std::set<std::string> validDevices;

private:
    std::string iniFileName;
};

#endif

