/*
    fcnffile.h


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

#ifndef FCNFFILE_INCLUDED
#define FCNFFILE_INCLUDED

#include "typedefs.h"
#include <optional>
#include <memory>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <filesystem>

namespace fs = std::filesystem;


struct sIoDeviceMapping
{
    std::string name;
    Word baseAddress{};
    std::optional<Word> byteSize;
};


class FlexemuConfigFile
{
public:
    FlexemuConfigFile() = delete;
    FlexemuConfigFile(const FlexemuConfigFile &src) = delete;
    FlexemuConfigFile(FlexemuConfigFile &&src) noexcept = default;
    explicit FlexemuConfigFile(fs::path p_path);
    ~FlexemuConfigFile() = default;

    FlexemuConfigFile &operator=(const FlexemuConfigFile &) = delete;
    FlexemuConfigFile &operator=(FlexemuConfigFile &&src) noexcept = default;

    fs::path GetPath() const;
    bool IsValid() const;
    std::vector<sIoDeviceMapping> GetIoDeviceMappings() const;
    std::string GetDebugSupportOption(const std::string &key) const;
    std::pair<std::string, std::set<std::string> > GetIoDeviceLogging() const;
    std::optional<Word> GetSerparAddress(const fs::path &monitorFilePath) const;
    std::optional<Byte> GetBootCharacter(const fs::path &monitorFilePath) const;

protected:
    void InitializeIoDeviceMappings();
    void InitializeDebugSupportOptions();
    void InitializeIoDeviceLogging();
    void InitializeSerparAddresses();
    void InitializeBootCharacters();

private:
    fs::path path;
    std::vector<sIoDeviceMapping> ioDeviceMappings;
    std::map<std::string, std::string> debugSupportOptionForKey;
    std::pair<std::string, std::set<std::string> > ioDeviceLogging;
    std::map<std::string, Word> serparAddressForMonitorFile;
    std::map<std::string, Byte> bootCharacterForMonitorFile;
};

using FlexemuConfigFileSPtr = std::shared_ptr<FlexemuConfigFile>;
#endif

