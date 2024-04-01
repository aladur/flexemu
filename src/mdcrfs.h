/*
    mdcrfs.h


    FLEXplorer, An explorer for any FLEX file or disk container
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
#include <vector>
#include <string>
#include <functional>

class BMemoryBuffer;
class MiniDcrTape;


enum class MdcrWriteMode
{
    Truncate, // Write the file at the beginning of the tape.
              // Any existing file is overwritten.
    Append,   // Append the file at the end of the tape.
};

enum class MdcrStatus
{
    Success,       // The function call was successfull.
    InvalidData,   // The input data is empty, invalid or could not be evaluated.
    WriteProtected,// The tape is write protected.
    WrongChecksum, // When reading a record the checksum is wrong.
    DoubleName,    // A file with the name already exists on the tape.
    ReadError,     // An error occurred when reading a record.
    WriteError,    // An error occurred when writing a record.
};

extern const std::vector<std::string> mdcrErrors;

class MdcrFileSystem
{
    private:
        static void SetFilename(std::vector<Byte>::iterator &iter,
                         const char *mdcrFilename);
        static std::string GetFilename(std::vector<Byte>::const_iterator &iter);
        static Byte CalculateChecksum(std::vector<Byte>::const_iterator &iter,
                               size_t size);

        const size_t MaxRecordSize = 1024;

    public:
        static std::string CreateMdcrFilename(const char *string,
                                              bool toUppercase);

        static MdcrStatus ReadFile(
                      std::string &filename,
                      BMemoryBuffer &memory,
                      MiniDcrTape &mdcr);
        MdcrStatus WriteFile(
                       const char *filename,
                       const BMemoryBuffer &memory,
                       MiniDcrTape &mdcr,
                       MdcrWriteMode mode,
                       bool toUppercase = true);
        MdcrStatus ForEachFile(MiniDcrTape &mdcr,
                          const std::function<MdcrStatus (const std::string&,
                                                    BMemoryBuffer &memory)>&);

    public:
        MdcrFileSystem() = default;
};

