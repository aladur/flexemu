/*
    mc146818.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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



#ifndef MC146818_INCLUDED
#define MC146818_INCLUDED

#include "misc1.h"
#include "iodevice.h"
#include "bobservd.h"


class Mc6809;

class Mc146818 : public IoDevice, public BObserved
{
    // Internal registers:

protected:
    Byte second;
    Byte minute;
    Byte hour;
    Byte al_second;
    Byte al_minute;
    Byte al_hour;
    Byte weekday;
    Byte day;
    Byte month;
    Byte year;
    Byte A;
    Byte B;
    Byte C;
    Byte D;
    Byte ram[50]; // 50 bytes of internal RAM

public:

    Byte readIo(Word offset) override;
    void writeIo(Word offset, Byte value) override;
    void resetIo() override;
    const char *getName() override
    {
        return "rtc";
    };
    Word sizeOfIo() override
    {
        return 64;
    };
    virtual void update_1_second();

private:

    Byte convert(Byte val);
    Byte convert_hour(Byte val);
    Byte convert_bin(Byte val);
    bool increment(Byte &, Byte, Byte);
    bool increment_hour(Byte &);
    bool increment_day(Byte &, Byte, Byte);
    const char          *getFileName();
    char path[PATH_MAX];

    // Public constructor and destructor

public:

    Mc146818();
    ~Mc146818() override;

};

#endif // MC146818_INCLUDED

