/*
    fcnffile.h


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

#ifndef FCNFFILE_INCLUDED
#define FCNFFILE_INCLUDED

#include "binifile.h"
#include <vector>
#include <string>
#include <set>
#include <map>


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
    FlexemuConfigFile(const FlexemuConfigFile &) = delete;
    FlexemuConfigFile(FlexemuConfigFile &&);
    FlexemuConfigFile(const char *aFileName);
    ~FlexemuConfigFile() = default;

    FlexemuConfigFile &operator=(const FlexemuConfigFile &) = delete;
    FlexemuConfigFile &operator=(FlexemuConfigFile &&);

    std::vector<sIoDeviceMapping> ReadIoDevices(
                                      std::set<std::string> validKeys);
    std::string GetDebugOption(const std::string &key);
    int GetSerparAddress(const std::string &monitorFilePath);

private:
    BIniFile iniFile;
};

#endif

