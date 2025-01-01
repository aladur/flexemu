/*
    rndcheck.h


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



#ifndef RNDCHECK_INCLUDED
#define RNDCHECK_INCLUDED

#include "filecnts.h"
#include <string>
#include <vector>

class RandomFileCheck
{
public:
    explicit RandomFileCheck(std::string p_directory);
    virtual ~RandomFileCheck();

    bool CheckForRandomAndUpdate(const std::string &filename);
    bool CheckForRandom(const std::string &filename) const;
    bool IsRandomFile(const std::string &filename) const;
    bool AddToRandomList(const std::string &filename);
    bool RemoveFromRandomList(const std::string &filename);
    bool CheckForFileAttributeAndUpdate(const std::string &filename);
    void CheckAllFilesAttributeAndUpdate();
    bool UpdateRandomListToFile();
    bool IsWriteProtected() const;

    static bool IsValidSectorMap(
            const SectorMap_t &sectorMap,
            uint32_t fileSize);

private:
    bool HasRandomFileAttribute(const std::string &filename) const;
    bool RemoveRandomFileAttribute(const std::string &filename);
    bool ReadRandomListFromFile(const std::string &path);
    bool WriteRandomListToFile();

    std::string directory;
    std::string randomListFile;
    // vector of lower case filenames of all random files:
    std::vector<std::string> randomFiles;
    bool isDirty{};
    bool isWriteProtected{};
};
#endif

