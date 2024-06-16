/*
    drisel.h


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


#ifndef DRISEL_INCLUDED
#define DRISEL_INCLUDED

#include "misc1.h"
#include "bytereg.h"

class E2floppy;

// DriveSelect emulates the Eurocom II drive select register,
// a single byte read/write register (SN74LS273, Octal
// D Flip-Flop with clear). Clear is connected with /RESET.
//
// Read:
//   Bit 1: Get current side
//   Bit 6: IRQ flag
//   Bit 7: Data request (DRQ) in progress
// Write:
//   Bit 0: Drive 0 selected
//   Bit 1: Drive 1 selected
//   Bit 2: Drive 2 selected
//   Bit 3: Drive 3 selected
//   Bit 4: Side 2 selected (Side 1 if low)
//
class DriveSelect : public ByteRegister
{
private:
    // Bits when reading register
    static const Byte SELECT_DRIVE0{0x01};
    static const Byte SELECT_DRIVE1{0x02};
    static const Byte SELECT_DRIVE2{0x04};
    static const Byte SELECT_DRIVE3{0x08};
    static const Byte WRITE_SIDE_MASK{0x10};

    // Bits when writing register
    static const Byte READ_SIDE_MASK{0x02};
    static const Byte READ_IRQ_MASK{0x40};
    static const Byte READ_DRQ_MASK{0x80};

    E2floppy &fdc;

    Byte requestReadValue() override;
    void requestWriteValue(Byte value) override;

public:

    DriveSelect() = delete;
    explicit DriveSelect(E2floppy &fdc);
    ~DriveSelect() override = default;

    void resetIo() override;
    const char *getName() override
    {
        return "drisel";
    }
};

#endif

