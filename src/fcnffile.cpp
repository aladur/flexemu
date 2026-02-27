/*
    fcnffile.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2026  W. Schwotzer

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

#include "typedefs.h"
#include "misc1.h"
#include "fcnffile.h"
#include "flexerr.h"
#include "binifile.h"
#include "filecntb.h"
#include "free.h"
#include <ios>
#include <utility>
#include <optional>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <filesystem>


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
        throw FlexException(FERR_UNABLE_TO_OPEN, GetPath());
    }

    InitializeIoDeviceMappings();
    InitializeDebugSupportOptions();
    InitializeIoDeviceLogging();
    InitializeSerparAddresses();
    InitializeBootCharacters();
    InitializeBootSectorFileProperties();
}

std::vector<sIoDeviceMapping> FlexemuConfigFile::GetIoDeviceMappings() const
{
    return ioDeviceMappings;
}

void FlexemuConfigFile::InitializeIoDeviceMappings()
{
    BIniFile iniFile(path);
    const std::string section{"IoDevices"};

    const auto valueForKey = iniFile.ReadSection(section);

    for (const auto &iter : valueForKey)
    {
        const auto &validDevices = GetValidDevices();
        if (validDevices.find(iter.first) == validDevices.cend())
        {
            const auto lineNumber = iniFile.GetLineNumber(section, iter.first);
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
            if (baseAddress > 0xFFFF)
            {
                const auto lineNumber =
                    iniFile.GetLineNumber(section, iter.first);
                throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                    lineNumber, iter.first + "=" + iter.second,
                                    iniFile.GetPath());
            }

            if (std::getline(stream, byteSizeString))
            {
                std::stringstream byteSizeStream;
                int byteSize;
                byteSizeStream << std::hex << byteSizeString;
                byteSizeStream >> byteSize;
                if (byteSize <= 0 || byteSize > 4096 ||
                        baseAddress + byteSize - 1U > 0xFFFF)
                {
                    const auto lineNumber =
                        iniFile.GetLineNumber(section, iter.first);
                    throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                        lineNumber,
                                        iter.first + "=" + iter.second,
                                        iniFile.GetPath());
                }
                mapping.byteSize = static_cast<Word>(byteSize);
            }

            mapping.baseAddress = static_cast<Word>(baseAddress);

            ioDeviceMappings.push_back(mapping);
            continue;
        }

        const auto lineNumber = iniFile.GetLineNumber(section, iter.first);
        throw FlexException(FERR_INVALID_LINE_IN_FILE,
                            lineNumber, iter.first + "=" + iter.second,
                            iniFile.GetPath());
    }
}

std::optional<Word> FlexemuConfigFile::GetSerparAddress(
        const fs::path &monitorFilePath) const
{
    auto filename = monitorFilePath.filename().u8string();

#ifdef _WIN32
    flx::strlower(filename);
#endif
    const auto iter = serparAddressForMonitorFile.find(filename);

    return (iter == serparAddressForMonitorFile.cend()) ?
        std::nullopt : std::optional<Word>(iter->second);
}

void FlexemuConfigFile::InitializeSerparAddresses()
{
    BIniFile iniFile(path);
    const std::string section{"SERPARAddress"};

    const auto valueForKey = iniFile.ReadSection(section);

    for (const auto &iter : valueForKey)
    {
        std::size_t address;
        auto key = iter.first;

#ifdef _WIN32
        flx::strlower(key);
#endif
        if (flx::convert(iter.second, address, 16) && address <= 0xFFFF)
        {
            serparAddressForMonitorFile.emplace(
                    key, static_cast<Word>(address));
            continue;
        }

        const auto lineNumber = iniFile.GetLineNumber(section, iter.first);
        throw FlexException(FERR_INVALID_LINE_IN_FILE,
                            lineNumber, iter.first + "=" + iter.second,
                            iniFile.GetPath());
    }
}

std::string FlexemuConfigFile::GetDebugSupportOption(const std::string &key)
    const
{
    const auto iter = debugSupportOptionForKey.find(key);

    return (iter == debugSupportOptionForKey.cend()) ? "" : iter->second;
}

void FlexemuConfigFile::InitializeDebugSupportOptions()
{
    static const auto validKeys = std::set<std::string>{
        "presetRAM", "logMdcr", "logMdcrFilePath"
    };

    BIniFile iniFile(path);
    const std::string section{"DebugSupport"};

    const auto valueForKey = iniFile.ReadSection(section);

    for (const auto &iter : valueForKey)
    {
        if (validKeys.find(iter.first) == validKeys.cend())
        {
            const auto lineNumber = iniFile.GetLineNumber(section, iter.first);
            throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                lineNumber, iter.first + "=" + iter.second,
                                iniFile.GetPath());
        }

        debugSupportOptionForKey.emplace(iter.first, iter.second);
    }
}

std::pair<std::string, std::set<std::string> >
    FlexemuConfigFile::GetIoDeviceLogging() const
{
    return ioDeviceLogging;
}

void FlexemuConfigFile::InitializeIoDeviceLogging()
{
    std::string logFilePath;
    std::set<std::string> devices;
    BIniFile iniFile(path);
    const std::string section{"IoDeviceLogging"};

    const auto valueForKey = iniFile.ReadSection(section);

    for (const auto &iter : valueForKey)
    {
        if (iter.first == "devices")
        {
            std::stringstream stream(iter.second);
            std::string device;

            while (std::getline(stream, device, ','))
            {
                device = flx::trim(std::move(device));

                const auto &validDevices = GetValidDevices();
                if (validDevices.find(device) != validDevices.cend())
                {
                    devices.emplace(device);
                    continue;
                }

                const auto lineNumber =
                    iniFile.GetLineNumber(section, iter.first);
                throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                    lineNumber, iter.first + "=" + iter.second,
                                    iniFile.GetPath());
            }

        }
        else if (iter.first == "logFilePath")
        {
            logFilePath = iter.second;
        }
        else
        {
            const auto lineNumber = iniFile.GetLineNumber(section, iter.first);
            throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                lineNumber, iter.first + "=" + iter.second,
                                iniFile.GetPath());
        }
    }

    if (!devices.empty())
    {
        if (logFilePath.empty())
        {
            logFilePath =
                (fs::temp_directory_path() / u8"flexemu_device.log").u8string();
        }

        ioDeviceLogging = std::pair(logFilePath, devices);
    }
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

std::optional<Byte> FlexemuConfigFile::GetBootCharacter(
        const fs::path &monitorFilePath) const
{
    auto filename = monitorFilePath.filename().u8string();

#ifdef _WIN32
    flx::strlower(filename);
#endif

    const auto iter = bootCharacterForMonitorFile.find(filename);

    return (iter == bootCharacterForMonitorFile.cend()) ?
        std::nullopt : std::optional<Byte>(iter->second);
}

void FlexemuConfigFile::InitializeBootCharacters()
{
    BIniFile iniFile(path);
    const std::string section{"BootCharacter"};

    const auto valueForKey = iniFile.ReadSection(section);

    for (const auto &iter : valueForKey)
    {
        auto key = iter.first;

#ifdef _WIN32
        flx::strlower(key);
#endif

        if (iter.second.size() != 1)
        {
            const auto lineNumber = iniFile.GetLineNumber(section, iter.first);
            throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                lineNumber, iter.first + "=" + iter.second,
                                iniFile.GetPath());
        }

        bootCharacterForMonitorFile.emplace(key, iter.second[0]);
    }
}

BootSectorFileProperties_t FlexemuConfigFile::GetBootSectorFileProperties()
    const
{
    return bootSectorFileProperties;
}

void FlexemuConfigFile::InitializeBootSectorFileProperties()
{
    BIniFile iniFile(path);
    const std::string section{"BootSectorFile"};
    static const std::regex regex("[a-z][a-z0-9_]{0,7}");
    std::set<std::string> addedBootSectorFiles;
    const auto valueForKey = iniFile.ReadSectionOrdered(section);

    for (const auto &iter : valueForKey)
    {
        std::string bootSectorFile = iter.first;
        std::size_t offset;

        if (regex_match(bootSectorFile, regex) &&
            addedBootSectorFiles.find(bootSectorFile) ==
                addedBootSectorFiles.cend() &&
            (flx::convert(iter.second, offset, 16) &&
             offset >= 0x02U && offset <= 0xFEU &&
            (bootSectorFile != BOOT_FILE || offset == 0x03U)))
        {
            bootSectorFileProperties.push_back(
                    {bootSectorFile, static_cast<Word>(offset)});

            addedBootSectorFiles.emplace(bootSectorFile);
            continue;
        }

        const auto lineNumber = iniFile.GetLineNumber(section, iter.first);
        throw FlexException(FERR_INVALID_LINE_IN_FILE,
                            lineNumber, iter.first + "=" + iter.second,
                            iniFile.GetPath());
    }

    // Default if section does not exist for backwards compatibility.
    if (bootSectorFileProperties.empty() && !iniFile.HasSection(section))
    {
        bootSectorFileProperties.push_back({BOOT_FILE, 0x03});
    }
}
