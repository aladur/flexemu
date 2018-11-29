/*
    pia2v5.h


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



#ifndef PIA2V5_INCLUDED
#define PIA2V5_INCLUDED

#include "misc1.h"
#include "e2.h"
#include "mc6821.h"
#include "mdcrtape.h"
#include <memory>
#include <array>
#include <vector>
#include <fstream>


class Mc6809;

class Pia2V5 : public Mc6821
{

    // Philips MDCR digital cassette recorder connected to Port A and B

private:
    enum class TapeDirection : Byte
    {
        Forward,
        Rewind,
        NONE,
    };

    enum class ReadMode : Byte
    {
        Init, // Delay until ready for read next record
        Read, // Reading a record
        Off,  // No read in progress
    };

    Mc6809 &cpu;

    Byte write_bit_mask;
    Byte write_byte;
    Byte last_ora;
    Byte read_bit_mask;
    Word read_index;
    std::vector<Byte> read_buffer;
    std::vector<Byte> write_buffer;
    MiniDcrTapePtr drive[2];
    int drive_idx;
    ReadMode read_mode;
    TapeDirection direction;
    Word debug;
    std::string disk_dir;
    std::fstream cdbg;
    // following delay/cycle variables/constants are multiples of cpu cycles
    QWord delay_RDC; // Delay until a read clock is present
    // Delay until a begin/end of tape is detected
    static const QWord delay_BET = (QWord)(2000.f / ORIGINAL_PERIOD);
    QWord cycles_RDC;
    QWord cycles_BET;
    QWord cycles_cdbg;

protected:
    void writeOutputA(Byte value) override;
    Byte readInputA() override;
    void requestInputA() override;
    void writeOutputB(Byte value) override;
    Byte readInputB() override;

public:
    void set_debug(const std::string &debugLevel, std::string logFilePath);
    void disk_directory(const char *x_disk_dir);
    void mount_all_drives(std::array<std::string, 2> drives);
    bool mount_drive(const char *path, Word drive_nr);

public:
    void resetIo() override;
    const char *getName() override
    {
        return "pia2";
    };

    Pia2V5(Mc6809 &x_cpu);
    virtual ~Pia2V5();

private:
    void log_buffer(const std::vector<Byte> &buffer);
    void SetReadModeToInit();
};

#endif // PIA2V5_INCLUDED


