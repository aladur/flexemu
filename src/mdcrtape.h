/*
    mdcrtape.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2018-2021  W. Schwotzer

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

#ifndef MDCRTAPE_INCLUDED
#define MDCRTAPE_INCLUDED

#include "misc1.h"
#include <memory>
#include <array>
#include <vector>
#include <fstream>


const int TYPE_MDCR_CONTAINER = 0x100; /* file container with MDCR format */

class MiniDcrTape;

using MiniDcrTapePtr = std::unique_ptr<MiniDcrTape>;

enum class RecordType : Byte
{
    NONE,      // There is no (more) record
    Header,    // Record containing a file header
    Data,      // Record containing file data
    LastData,  // Last record of a file containing file data
};

class MiniDcrTape
{
    enum class Mode : Byte
    {
        Create, // Create a new file
        Open,   // Open an existing file
    };

private:
// Using a Byte stream would fit better but no read/write is possible
// on Linux gcc 5.4.0
//    std::basic_fstream<Byte> stream
    std::fstream stream;
    bool is_write_protected;
    std::ios::pos_type max_pos;
    std::vector<std::ios::pos_type> record_positions;
    std::vector<RecordType> record_types;
    DWord record_index;

public:
    MiniDcrTape() = delete;
    MiniDcrTape(const MiniDcrTape &) = delete;
    MiniDcrTape(MiniDcrTape &&) = delete;
    MiniDcrTape(const char *path, Mode mode);
    virtual ~MiniDcrTape();

    MiniDcrTape &operator= (const MiniDcrTape &) = delete;
    MiniDcrTape &operator= (MiniDcrTape &&) = delete;

    static MiniDcrTapePtr Create(const char *path);
    static MiniDcrTapePtr Open(const char *path);
    bool Close();
    bool IsOpen() const;
    bool HasRecord() const;
    RecordType GetRecordType() const;
    DWord GetRecordIndex() const;
    bool ReadRecord(std::vector<Byte> &buffer);
    bool WriteRecord(const std::vector<Byte> &buffer);
    bool GotoPreviousRecord();
    bool IsWriteProtected() const;

    static const std::array<char, 4> magic_bytes;

private:
    bool VerifyTape();

};  // class MiniDcrTape

#endif // MDCRTAPE_INCLUDED
