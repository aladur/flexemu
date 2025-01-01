/*
    mc146818.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

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
#include <string>
#include <array>


class Mc6809;

class Mc146818 : public IoDevice, public BObserved
{
    // Internal registers:

protected:
    Byte second{0};
    Byte minute{0};
    Byte hour{0};
    Byte al_second{0};
    Byte al_minute{0};
    Byte al_hour{0};
    Byte weekday{0};
    Byte day{0};
    Byte month{0};
    Byte year{0};
    Byte A{0};
    Byte B{0};
    Byte C{0};
    Byte D{0};
    std::array<Byte, 50> ram{}; // 50 bytes of internal RAM

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

    enum class Config : std::uint8_t
    {
        NewOrOld, // Use new config path if exists, otherwise old path.
        Old, // Always use new config path.
        New, // Always use new config path.
    };

    Byte convert(Byte val) const;
    Byte convert_hour(Byte val) const;
    Byte convert_bin(Byte val) const;
    bool increment(Byte &reg, Byte min, Byte max);
    bool increment_hour(Byte &p_hour) const;
    bool increment_day(Byte &p_day, Byte p_month, Byte p_year);

    static std::string getConfigFilePath(Config type);

public:

    Mc146818();
    ~Mc146818() override;

};

#endif // MC146818_INCLUDED

