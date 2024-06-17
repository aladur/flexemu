/*
    fcnffile.cpp


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

#include "misc1.h"
#include "fcnffile.h"
#include "flexerr.h"
#include "binifile.h"
#include "e2.h"
#include <sstream>
#include <algorithm>


const std::set<std::string> FlexemuConfigFile::validDevices =
    std::set<std::string>{
        "mmu", "acia1", "pia1", "pia2", "fdc",
        "drisel", "command", "vico1", "vico2", "rtc"
    };

FlexemuConfigFile::FlexemuConfigFile(const char *aFileName) :
     iniFileName(aFileName)
{
    BIniFile iniFile(iniFileName.c_str());

    if (!iniFile.IsValid())
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, aFileName);
    }
}

FlexemuConfigFile::FlexemuConfigFile(FlexemuConfigFile &&src) noexcept :
    iniFileName(std::move(src.iniFileName))
{
}

FlexemuConfigFile &FlexemuConfigFile::operator=(FlexemuConfigFile &&src)
noexcept
{
    iniFileName = std::move(src.iniFileName);

    return *this;
}

std::vector<sIoDeviceMapping> FlexemuConfigFile::ReadIoDevices()
{
    std::vector<sIoDeviceMapping> deviceMappings;
    BIniFile iniFile(iniFileName.c_str());

    auto valueForKey = iniFile.ReadSection("IoDevices");

    for (const auto &iter : valueForKey)
    {
        if (validDevices.find(iter.first) == validDevices.end())
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

            mapping.baseAddress = static_cast<Word>(baseAddress);

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

int FlexemuConfigFile::GetSerparAddress(const std::string &monitorFilePath)
{
    std::string fileName = flx::getFileName(monitorFilePath);

#ifdef _WIN32
    flx::strlower(fileName);
#endif

    BIniFile iniFile(iniFileName.c_str());

    auto valueForKey = iniFile.ReadSection("SERPARAddress");

    for (const auto &iter : valueForKey)
    {
        std::string key = iter.first;

#ifdef _WIN32
        flx::strlower(key);
#endif
        if (fileName == key)
        {
            std::stringstream stream(iter.second);

            std::string addressString;

            if (std::getline(stream, addressString))
            {
                int address;
                std::stringstream addressStream;

                addressStream << std::hex << addressString;
                addressStream >> address;
                if (address < 0 || address > 0xffff)
                {
                    throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                        iter.first + "=" + iter.second,
                                        iniFile.GetFileName());
                }

                return address;
            }

            throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                iter.first + "=" + iter.second,
                                iniFile.GetFileName());
        }
    }

    return -1;
}

std::string FlexemuConfigFile::GetDebugSupportOption(const std::string &key)
{
    static const auto validKeys = std::set<std::string>{
        "presetRAM", "logMdcr", "logMdcrFilePath"
    };

    BIniFile iniFile(iniFileName.c_str());

    auto valueForKey = iniFile.ReadSection("DebugSupport");

    for (const auto &iter : valueForKey)
    {
        if (!validKeys.empty() && validKeys.find(iter.first) == validKeys.end())
        {
            throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                iter.first + "=" + iter.second,
                                iniFile.GetFileName());
        }

        if (iter.first == key)
        {
            return iter.second;
        }
    }

    return {};
}

std::pair<std::string, std::set<std::string> >
    FlexemuConfigFile::GetIoDeviceLogging()
{
    static const auto validKeys = std::set<std::string>{
        "logFilePath", "devices"
    };

    std::string logFilePath;
    std::set<std::string> devices;
    std::pair<std::string, std::set<std::string> > result;
    BIniFile iniFile(iniFileName.c_str());

    auto valueForKey = iniFile.ReadSection("IoDeviceLogging");

    for (const auto &iter : valueForKey)
    {
        if (!validKeys.empty() && validKeys.find(iter.first) == validKeys.end())
        {
            throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                iter.first + "=" + iter.second,
                                iniFile.GetFileName());
        }

        if (iter.first == "devices")
        {
            std::stringstream stream(iter.second);
            std::string device;

            while (std::getline(stream, device, ','))
            {
                device = flx::trim(std::move(device));

                if (validDevices.find(device) == validDevices.end())
                {
                    throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                iter.first + "=" + iter.second,
                                iniFile.GetFileName());
                }

                devices.emplace(device);
            }

        }
        else if (iter.first == "logFilePath")
        {
            logFilePath = iter.second;
        }
    }

    if (!devices.empty())
    {
        if (logFilePath.empty())
        {
            logFilePath = flx::getTempPath() + "/flexemu_device.log";
        }

        result.first = logFilePath;
        result.second = devices;
    }

    return result;
}

