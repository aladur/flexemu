/*
    fcnffile.cpp


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

#include "misc1.h"
#include "fcnffile.h"
#include "flexerr.h"
#include "e2.h"
#include <sstream>


FlexemuConfigFile::FlexemuConfigFile(const char *aFileName) :
     iniFile(aFileName)
{
    if (!iniFile.IsValid())
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, aFileName);
    }
}

FlexemuConfigFile::FlexemuConfigFile(FlexemuConfigFile &&src) :
    iniFile(std::move(src.iniFile))
{
}

FlexemuConfigFile &FlexemuConfigFile::operator=(FlexemuConfigFile &&src) 
{
    iniFile = std::move(src.iniFile);

    return *this;
}

std::vector<sIoDeviceMapping> FlexemuConfigFile::ReadIoDevices(
                                            std::set<std::string> validKeys)
{
    std::vector<sIoDeviceMapping> deviceMappings;

    auto valueForKey = iniFile.ReadSection("IoDevices");

    for (const auto iter : valueForKey)
    {
        if (!validKeys.empty() && validKeys.find(iter.first) == validKeys.end())
        {
            throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                iter.first + "=" + iter.second,
                                iniFile.GetFileName());
        }

        std::stringstream stream(iter.second);

        std::string addressString;
        std::string byteSizeString;
        sIoDeviceMapping mapping;

        if (std::getline(stream, addressString, ','))
        {
            unsigned int baseAddress;
            std::stringstream addressStream;

            mapping.name = iter.first;
            mapping.byteSize = -1;
            addressStream << std::hex << addressString;
            addressStream >> baseAddress;
            if (baseAddress < GENIO_BASE || baseAddress > 0xffff)
            {
                throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                    iter.first + "=" + iter.second,
                                    iniFile.GetFileName());
            }

            if (std::getline(stream, byteSizeString))
            {
                std::stringstream byteSizeStream;
                byteSizeStream << std::hex << byteSizeString;
                byteSizeStream >> mapping.byteSize;
                if (mapping.byteSize <= 0 || mapping.byteSize > 64)
                {
                    throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                        iter.first + "=" + iter.second,
                                        iniFile.GetFileName());
                }
            }

            mapping.baseAddress = baseAddress;

            deviceMappings.push_back(mapping);
        }
        else
        {
            throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                iter.first + "=" + iter.second,
                                iniFile.GetFileName());
        }
    }

    return deviceMappings;
}

