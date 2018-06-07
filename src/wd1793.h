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
public:

    const Byte STR_NOTREADY{0x80};
    const Byte STR_PROTECTED{0x40};
    const Byte STR_HEADLOADED{0x20};
    const Byte STR_RECORDNOTFOUND{0x10}; // for type 2 and 3 commands
    const Byte STR_SEEKERROR{0x10}; // for type 1 commands
    const Byte STR_TRACK0{0x04};
    const Byte STR_DATAREQUEST{0x02}; // for type 2 and 3 commands
    const Byte STR_INDEX{0x02}; // for type 1 commands
    const Byte STR_BUSY{0x01};

    // Internal registers:
    //
    // dr       data register (r/w)
    // tr       track register (r/w)
    // sr       sector register (r/w)
    // cr       command register (w)
    // str      status register (r)
    // isStepIn flag indicating that previous step command was STEP IN
    // side     side of floppy to read/write on, changeable by subclass
    // drq      status of drq pin (= data request)
    // irq      status of irq pin (= interrupt)
    // byteCount    byte counter during read/write
    // strRead  count read access to command register, reset by read from dr

private:

    Byte                dr, tr, sr, cr, str;
    bool                isStepIn;
    bool                isDataRequest, isInterrupt;
    Byte                side;
    unsigned int            byteCount, strRead;

    // Internal functions
private:

    void                 do_seek(Byte new_track);

protected:

    void setIrq()
    {
        isInterrupt = true;
    }

    void resetIrq()
    {
        isInterrupt = false;
        // nterrupt request to CPU
    }

    bool isIrq() const
    {
        return isInterrupt;
    }

    bool isDrq() const
    {
        return isDataRequest;
    }

    Byte getSide() const
    {
        return side;
    }

    void setSide(Byte newSide)
    {
        side = newSide;
    }

    Byte getTrack() const
    {
        return tr;
    }

    Byte getSector() const
    {
        return sr;
    }

    void setTrack(Byte newTrack)
    {
        tr = newTrack;
    }

    void setSector(Byte newSector)
    {
        sr = newSector;
    }

    Byte getDataRegister() const
    {
        return dr;
    }

    void command(Byte command);
    void                     setStatusRecordNotFound();
    bool                     isErrorStatus();

    virtual bool             driveReady();
    virtual bool             seekError(Byte new_track);
    virtual bool             writeProtect();
    virtual bool             recordNotFound();
    virtual Byte             readByte(Word index);
    virtual void             writeByte(Word index);

    // Read and write functions

public:

    virtual void             resetIo();
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

