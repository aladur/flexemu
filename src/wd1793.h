/*
    wd1793.h


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


#ifndef __wd1793_h__
#define __wd1793_h__

#include "misc1.h"
#include <stdio.h>

#include "iodevice.h"
#include "filecntb.h"   // needed only for SECTOR_SIZE


class Wd1793 : public IoDevice
{

    // Internal registers:
    //
    // dr       data register (r/w)
    // tr       track register (r/w)
    // sr       sector register (r/w)
    // cr       command register (w)
    // str      status register (r)
    // isStepIn flag indicating that previous step command was STEP IN
    // side     side of floppy to read/write on, changeable by subclass
    // drq      status of drq pin
    // irq      status of irq pin
    // byteCount    byte counter during read/write
    // strRead  count read access to command register, reset by read from dr

protected:

    Byte                dr, tr, sr, cr, str;
    Byte                isStepIn, drq, irq, side;
    unsigned int            byteCount, strRead;

    // Internal functions
private:

    void                 do_seek(Byte new_track);

protected:

    virtual void             setIrq(void);
    virtual void             resetIrq(void);
    virtual Byte             driveReady(void);
    virtual Byte             seekError(Byte new_track);
    virtual Byte             writeProtect(void);
    virtual Byte             recordNotFound(void);
    virtual void             command(Byte command);
    virtual Byte             readByte(Word index);
    virtual void             writeByte(Word index);

    // Read and write functions

public:

    virtual void             resetIo(void);
    virtual Byte             readIo(Word offset);
    virtual void             writeIo(Word offset, Byte val);
    virtual const char      *getName()
    {
        return "wd1793";
    };
    // Public constructor and destructor

public:

    Wd1793();
    virtual             ~Wd1793();

};

#endif // __wd1793_h__

