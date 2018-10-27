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

    Mc6809 &cpu;

    Byte last_ora;
    DWord delay_BET;
    Byte read_bit_mask;
    Word read_index;
    unsigned int delay_read;
    Byte write_bit_mask;
    Byte write_byte;
    std::vector<Byte> read_buffer;
    std::vector<Byte> write_buffer;
    MiniDcrTapePtr drive[2];
    int drive_idx;
    bool read_mode;
    TapeDirection direction;
    static const DWord default_delay_BET = 50;
    std::string disk_dir;
    Word debug;
    std::fstream cdbg;

protected:

    void writeOutputA(Byte value) override;
    Byte readInputA() override;
    void requestInputA() override;
    void writeOutputB(Byte value) override;
    Byte readInputB() override;

public:
    void set_debug(const std::string &debugLevel,
                   std::string logFilePath);
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
};

#endif // PIA2V5_INCLUDED


