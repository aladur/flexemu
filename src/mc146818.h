/*
    mc146818.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

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



#ifndef __mc146818_h__
#define __mc146818_h__

#include "misc1.h"
#include "iodevice.h"


class Mc6809;
class Inout;

class Mc146818 : public IoDevice
{

    // Internal registers:
    //

protected:
    Mc6809                 *cpu;
    Inout                  *io;

    Byte                second, minute, hour;
    Byte                al_second, al_minute, al_hour;
    Byte                weekday, day, month, year;
    Byte                A, B, C, D;
    Byte                ram[50]; // 50 bytes of internal RAM

public:

    virtual Byte             readIo(Word offset);
    virtual void             writeIo(Word offset, Byte val);
    virtual void             resetIo();
    virtual const char      *getName()
    {
        return "mc146818";
    };
    virtual void             update_1_second();

private:

    Byte                convert(Byte val);
    Byte                convert_hour(Byte val);
    Byte                convert_bin(Byte val);
    Byte                increment(Byte *, Byte, Byte);
    Byte                increment_hour(Byte *);
    Byte                increment_day(Byte *, Byte, Byte);
    const char          *getFileName();
    char                path[PATH_MAX];

    // Public constructor and destructor

public:

    Mc146818(Inout *x_io, Mc6809 *x_cpu);
    virtual             ~Mc146818();

};

#endif // __mc146818_h__

