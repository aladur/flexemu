/*
    fcnffile.cpp


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

#include "misc1.h"
#include "fcnffile.h"
#include "flexerr.h"
#include "binifile.h"
#include "e2.h"
#include <sstream>
#include <algorithm>


static const std::set<std::string> &GetValidDevices()
{
    static const std::set<std::string> validDevices
    {
         "mmu",
         "acia1",
         "pia1",
         "pia2",
         "fdc",
         "drisel",
         "command",
         "vico1",
         "vico2",
         "rtc",
         "tstdev",
    };

    return validDevices;
}

FlexemuConfigFile::FlexemuConfigFile(fs::path p_path) :
     path(std::move(p_path))
{
    BIniFile iniFile(path);

    if (!iniFile.IsValid())
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, path.u8string());
    }
}

FlexemuConfigFile::FlexemuConfigFile(FlexemuConfigFile &&src) noexcept :
    path(std::move(src.path))
{
}

FlexemuConfigFile &FlexemuConfigFile::operator=(FlexemuConfigFile &&src)
noexcept
{
    path = std::move(src.path);

    return *this;
}

std::vector<sIoDeviceMapping> FlexemuConfigFile::ReadIoDevices() const
{
    std::vector<sIoDeviceMapping> deviceMappings;
    BIniFile iniFile(path);
    const std::string section{"IoDevices"};

    auto valueForKey = iniFile.ReadSection(section);

    for (const auto &iter : valueForKey)
    {
        const auto &validDevices = GetValidDevices();
        if (validDevices.find(iter.first) == validDevices.end())
        {
            auto lineNumber = iniFile.GetLineNumber(section, iter.first);
            throw FlexException(FERR_INVALID_LINE_IN_FILE,
                        lineNumber, iter.first + "=" + iter.second,
                        iniFile.GetPath());
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
            addressStream << std::hex << addressString;
            addressStream >> baseAddress;
            if (baseAddress > 0xffff)
            {
                auto lineNumber = iniFile.GetLineNumber(section, iter.first);
                throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                    lineNumber,
                                    iter.first + "=" + iter.second,
                                    iniFile.GetPath());
            }

            if (std::getline(stream, byteSizeString))
            {
                std::stringstream byteSizeStream;
                int byteSize;
                byteSizeStream << std::hex << byteSizeString;
                byteSizeStream >> byteSize;
                if (byteSize <= 0 || byteSize > 4096 ||
                        baseAddress + byteSize - 1U > 0xffff)
                {
                    auto lineNumber =
                        iniFile.GetLineNumber(section, iter.first);
                    throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                        lineNumber,
                                        iter.first + "=" + iter.second,
                                        iniFile.GetPath());
                }
                mapping.byteSize = static_cast<Word>(byteSize);
            }

            mapping.baseAddress = static_cast<Word>(baseAddress);

            deviceMappings.push_back(mapping);
        }
        else
        {
            auto lineNumber = iniFile.GetLineNumber(section, iter.first);
            throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                lineNumber, iter.first + "=" + iter.second,
                                iniFile.GetPath());
        }
    }

    return deviceMappings;
}

int FlexemuConfigFile::GetSerparAddress(const fs::path &monitorFilePath)
    const
{
    std::string fileName = monitorFilePath.filename().u8string();

#ifdef _WIN32
    flx::strlower(fileName);
#endif

    BIniFile iniFile(path);
    const std::string section{"SERPARAddress"};

    auto valueForKey = iniFile.ReadSection(section);

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
                    auto lineNumber =
                        iniFile.GetLineNumber(section, iter.first);
                    throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                        lineNumber,
                                        iter.first + "=" + iter.second,
                                        iniFile.GetPath());
                }

                return address;
            }

            auto lineNumber = iniFile.GetLineNumber(section, iter.first);
            throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                lineNumber,
                                iter.first + "=" + iter.second,
                                iniFile.GetPath());
        }
    }

    return -1;
}

std::string FlexemuConfigFile::GetDebugSupportOption(const std::string &key)
    const
{
    static const auto validKeys = std::set<std::string>{
        "presetRAM", "logMdcr", "logMdcrFilePath"
    };

    BIniFile iniFile(path);
    const std::string section{"DebugSupport"};

    auto valueForKey = iniFile.ReadSection(section);

    for (const auto &iter : valueForKey)
    {
        if (validKeys.find(iter.first) == validKeys.end())
        {
            auto lineNumber = iniFile.GetLineNumber(section, iter.first);
            throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                lineNumber, iter.first + "=" + iter.second,
                                iniFile.GetPath());
        }

        if (iter.first == key)
        {
            return iter.second;
        }
    }

    return {};
}

std::pair<std::string, std::set<std::string> >
    FlexemuConfigFile::GetIoDeviceLogging() const
{
    static const auto validKeys = std::set<std::string>{
        "logFilePath", "devices"
    };

    std::string logFilePath;
    std::set<std::string> devices;
    std::pair<std::string, std::set<std::string> > result;
    BIniFile iniFile(path);
    const std::string section{"IoDeviceLogging"};

    auto valueForKey = iniFile.ReadSection(section);

    for (const auto &iter : valueForKey)
    {
        if (validKeys.find(iter.first) == validKeys.end())
        {
            auto lineNumber = iniFile.GetLineNumber(section, iter.first);
            throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                lineNumber, iter.first + "=" + iter.second,
                                iniFile.GetPath());
        }

        if (iter.first == "devices")
        {
            std::stringstream stream(iter.second);
            std::string device;

            while (std::getline(stream, device, ','))
            {
                device = flx::trim(std::move(device));

                const auto &validDevices = GetValidDevices();
                if (validDevices.find(device) == validDevices.end())
                {
                    auto lineNumber =
                        iniFile.GetLineNumber(section, iter.first);
                    throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                lineNumber,
                                iter.first + "=" + iter.second,
                                iniFile.GetPath());
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
            logFilePath =
                (flx::getTempPath() / u8"flexemu_device.log").u8string();
        }

        result.first = logFilePath;
        result.second = devices;
    }

    return result;
}

fs::path FlexemuConfigFile::GetPath() const
{
    return path;
}

bool FlexemuConfigFile::IsValid() const
{
    BIniFile iniFile(path);

    return iniFile.IsValid();
}

Byte FlexemuConfigFile::GetBootCharacter(const fs::path &monitorFilePath)
    const
{
    std::string fileName = monitorFilePath.filename().u8string();
    Byte result{};

#ifdef _WIN32
    flx::strlower(fileName);
#endif

    BIniFile iniFile(path);
    const std::string section{"BootCharacter"};

    auto valueForKey = iniFile.ReadSection(section);

    for (const auto &iter : valueForKey)
    {
        std::string key = iter.first;

#ifdef _WIN32
        flx::strlower(key);
#endif
        if (fileName == key)
        {
            if (iter.second.size() != 1)
            {
                    auto lineNumber =
                        iniFile.GetLineNumber(section, iter.first);
                    throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                        lineNumber,
                                        iter.first + "=" + iter.second,
                                        iniFile.GetPath());
            }
            result = static_cast<Byte>(iter.second[0]);
        }
    }

    return result;
}
