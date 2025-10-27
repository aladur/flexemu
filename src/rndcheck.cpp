/*
    rndcheck.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2025  W. Schwotzer

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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#if defined(UNIX) || defined(USE_CMAKE)
#include "config.h"
#else
#include "confignt.h"
#endif
#include "typedefs.h"
#include "misc1.h"
#include "rndcheck.h"
#include "filecnts.h"
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef _WIN32
#include "cvtwchar.h"
#endif
#include <cstdint>
#include <system_error>
#include <utility>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#ifdef _WIN32
#include "windefs.h"
#endif

namespace fs = std::filesystem;

const char * const RANDOM_FILE_LIST = u8"random";
const char * const RANDOM_FILE_LIST_NEW = u8".random";

RandomFileCheck::RandomFileCheck(fs::path p_directory)
    : directory(std::move(p_directory))
{
    fs::path path;
    int idx = 0;

    for (const auto *file : { RANDOM_FILE_LIST_NEW, RANDOM_FILE_LIST })
    {
        const auto test_path = directory / file;

        if (fs::exists(test_path))
        {
            path = test_path;
            randomListFile = file;
            break;
        }
        ++idx;
    }

    if (idx == 1)
    {
        // If file random exists but directory does not allow to create new
        // files, like .random set read-only state.

        isWriteProtected =
#ifdef _WIN32
            (_waccess(directory.wstring().c_str(), W_OK) != 0) ?
#else
            (access(directory.u8string().c_str(), W_OK) != 0) ?
#endif
                true : isWriteProtected;
    }

    if (!path.empty())
    {
        std::fstream fs(path, std::ios::in | std::ios::out);

        isWriteProtected = !fs.is_open() ? true : isWriteProtected;
        fs.close();

        ReadRandomListFromFile(path);

        // In general trust the contents of the randomFiles. Just do a quick
        // check if file exists and it is between the mimimum and maximum
        // filesize of a random file.
        auto iter = std::remove_if(randomFiles.begin(), randomFiles.end(),
                [&](const auto &filename){
            std::error_code error;
            const auto file_path = directory / filename;
            const auto file_size = fs::file_size(file_path, error);

            return error ||
                file_size <= (2 * DBPS) ||
                file_size > (MAX_RANDOM_FILE_SECTORS * DBPS);
        });
        randomFiles.erase(iter, randomFiles.end());

        WriteRandomListToFile();

        if (!isDirty && idx == 1 &&
#ifdef _WIN32
                _waccess(directory.wstring().c_str(), W_OK) == 0 &&
                _waccess(path.wstring().c_str(), W_OK) == 0)
#else
                access(directory.u8string().c_str(), W_OK) == 0 &&
                access(path.u8string().c_str(), W_OK) == 0)
#endif
        {
            // Remove old random list file.
            fs::remove(path);
        }
    }
}

RandomFileCheck::~RandomFileCheck()
{
    UpdateRandomListToFile();
}

// Detailed verification of a potential random file by
// checking for a valid sector map.
// If file is verified as random file update randomFiles list.
bool RandomFileCheck::CheckForRandom(const std::string &filename) const
{
    const auto path = directory / flx::tolower(filename);
    bool result = false;
    std::error_code error;
    const auto file_size = fs::file_size(path, error);
    if (error || file_size <= (2 * DBPS))
    {
        return result;
    }

    if (IsRandomFile(filename))
    {
        return true;
    }

    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    SectorMap_t sectorMap{};

    if (ifs.is_open())
    {
        ifs.read(reinterpret_cast<char *>(sectorMap.data()),
                sectorMap.size());
        result = !ifs.fail() && IsValidSectorMap(sectorMap, file_size);
    }

    return result;
}

bool RandomFileCheck::CheckForRandomAndUpdate(const std::string &filename)
{
    if (CheckForRandom(filename))
    {
        AddToRandomList(filename);
        return true;
    }

    RemoveFromRandomList(filename);
    if (HasRandomFileAttribute(filename))
    {
        RemoveRandomFileAttribute(filename);
    }

    return false;
}

// Only check for file attributes if there is not yet a random file list
// available.
bool RandomFileCheck::CheckForFileAttributeAndUpdate(
        const std::string &filename)
{
    if (randomListFile.empty())
    {
        // Force writing random list file even if it is empty.
        isDirty = true;

        if (HasRandomFileAttribute(filename))
        {
            return CheckForRandomAndUpdate(filename);
        }
    }

    return false;
}

bool RandomFileCheck::IsRandomFile(const std::string &filename) const
{
    return std::binary_search(randomFiles.cbegin(), randomFiles.cend(),
            flx::tolower(filename));
}

bool RandomFileCheck::AddToRandomList(const std::string &filename)
{
    auto lcFilename = flx::tolower(filename);
    auto iter = std::find(randomFiles.cbegin(), randomFiles.cend(), lcFilename);

    if (iter == randomFiles.cend())
    {
        iter = std::lower_bound(randomFiles.begin(), randomFiles.end(),
                lcFilename);
        randomFiles.insert(iter, lcFilename);
        isDirty = true;
        return true;
    }

    return false;
}

bool RandomFileCheck::RemoveFromRandomList(const std::string &filename)
{
    const auto lcFilename = flx::tolower(filename);
    auto iter = std::find(randomFiles.cbegin(), randomFiles.cend(), lcFilename);

    if (iter != randomFiles.cend())
    {
        randomFiles.erase(iter);
        isDirty = true;
        return true;
    }

    return false;
}

bool RandomFileCheck::IsValidSectorMap(const SectorMap_t &sectorMap,
        std::uintmax_t fileSize)
{
    auto count = sectorMap.size() / 3;
    st_t trk_sec{};
    Byte sectorCount = 0U;
    uint32_t sectorSum = 0U;
    const auto expectedSectors = (fileSize - (2 * DBPS) + DBPS - 1) / DBPS;
    bool hasAllSectorsFound = false;
    // NOLINTNEXTLINE(readability-qualified-auto)
    auto iter = sectorMap.cbegin();

    // A random file has at least three sectors.
    if (expectedSectors == 0U)
    {
        return false;
    }

    while (count != 0U)
    {
        trk_sec.trk = *(iter++);
        trk_sec.sec = *(iter++);
        sectorCount = *(iter++);

        if (hasAllSectorsFound)
        {
            if (trk_sec != st_t{} || sectorCount != 0U)
            {
                return false;
            }
        }
        else
        {
            if (trk_sec.sec == 0U || sectorCount == 0U)
            {
                return false;
            }
            sectorSum += sectorCount;
            hasAllSectorsFound = (sectorSum >= expectedSectors);
        }

        --count;
    }

    return hasAllSectorsFound && (sectorSum == expectedSectors);
}

bool RandomFileCheck::HasRandomFileAttribute(const std::string &filename) const
{
    const auto path = directory / flx::tolower(filename);

#ifdef _WIN32
    DWord attributes = GetFileAttributes(path.wstring().c_str());

    return (attributes != INVALID_FILE_ATTRIBUTES &&
            (attributes & FILE_ATTRIBUTE_HIDDEN) != 0U);
#endif
#ifdef UNIX
    const auto status = fs::status(path);
    if (fs::is_regular_file(status) || fs::is_symlink(status))
    {
        return ((status.permissions() & fs::perms::owner_exec) ==
                fs::perms::owner_exec);
    }
#endif

    return false;
}

bool RandomFileCheck::RemoveRandomFileAttribute(const std::string &filename)
{
    const auto path = directory / flx::tolower(filename);

#ifdef _WIN32
    const auto wPath = path.wstring();
    DWord attributes = GetFileAttributes(wPath.c_str());

    if (attributes != INVALID_FILE_ATTRIBUTES)
    {
        attributes &= ~FILE_ATTRIBUTE_HIDDEN;
        return SetFileAttributes(wPath.c_str(), attributes) != 0;
    }
#endif
#ifdef UNIX
    const auto exec_perms =
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec;
    fs::permissions(path, exec_perms, fs::perm_options::remove);
    return true;
#endif

    return false;
}

bool RandomFileCheck::ReadRandomListFromFile(const fs::path &path)
{
    std::ifstream ifs(path);

    randomFiles.clear();

    if (ifs.is_open())
    {
        std::string line;

        while (std::getline(ifs, line))
        {
            const auto filename = flx::tolower(flx::trim(line));
            if (!filename.empty())
            {
                const auto iter = std::lower_bound(randomFiles.cbegin(),
                        randomFiles.cend(), filename);
                randomFiles.insert(iter, filename);
                isDirty = true;
            }
        }
        return true;
    }

    return false;
}

bool RandomFileCheck::WriteRandomListToFile()
{
    const auto path = directory / (randomListFile.empty() ?
        RANDOM_FILE_LIST_NEW : randomListFile);

    if (!isDirty || isWriteProtected)
    {
        return false;
    }

    std::ofstream ofs(path);

    if (ofs.is_open())
    {
        for (const auto &filename : randomFiles)
        {
            ofs << filename << "\n";
            if (HasRandomFileAttribute(filename))
            {
                RemoveRandomFileAttribute(filename);
            }
        }
        isDirty = false;
        randomListFile = RANDOM_FILE_LIST_NEW;
        return true;
    }

    return false;
}

bool RandomFileCheck::UpdateRandomListToFile()
{
    return WriteRandomListToFile();
}

bool RandomFileCheck::IsWriteProtected() const
{
    return isWriteProtected;
}


void RandomFileCheck::CheckAllFilesAttributeAndUpdate()
{
    fs::path path;

    if (!randomListFile.empty())
    {
        return;
    }

#ifdef _WIN32
    WIN32_FIND_DATA pentry;
    const auto wWildcard((directory / u8"*.*").wstring());

    auto hdl = FindFirstFile(wWildcard.c_str(), &pentry);

    if (hdl == INVALID_HANDLE_VALUE)
    {
        return;
    }

    do
    {
        auto filename(flx::tolower(ConvertToUtf8String(pentry.cFileName)));
        if (!flx::isFlexFilename(filename))
        {
            continue;
        }

        path = directory / filename;
        const auto status = fs::status(path);
        if (!fs::exists(status) || !fs::is_regular_file(status))
        {
            continue;
        }
        CheckForFileAttributeAndUpdate(filename);
    }
    while (FindNextFile(hdl, &pentry) != 0);

    FindClose(hdl);
#endif

#ifdef UNIX
    struct dirent *pentry;

    auto *pd = opendir(directory.u8string().c_str());
    if (pd == nullptr)
    {
        return;
    }

    while ((pentry = readdir(pd)) != nullptr)
    {
        std::string filename = pentry->d_name;
        if (!flx::isFlexFilename(filename))
        {
            continue;
        }

        path = directory / filename;
        const auto status = fs::status(path);
        if (!fs::exists(status) || !fs::is_regular_file(status))
        {
            continue;
        }
        CheckForFileAttributeAndUpdate(filename);
    }

    closedir(pd);
#endif
}
