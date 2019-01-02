/*
    wd1793.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2019  W. Schwotzer

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


#ifndef WD1793_INCLUDED
#define WD1793_INCLUDED

#include "misc1.h"
#include <stdio.h>

#include "iodevice.h"
#include "filecntb.h"   // needed only for SECTOR_SIZE


class Wd1793 : public IoDevice
{
public:

    // Bits of status register
    static const Byte STR_NOTREADY{0x80};
    static const Byte STR_PROTECTED{0x40};
    static const Byte STR_HEADLOADED{0x20};
    static const Byte STR_RECORDNOTFOUND{0x10}; // for type 2 and 3 commands
    static const Byte STR_SEEKERROR{0x10}; // for type 1 commands
    static const Byte STR_TRACK0{0x04};
    static const Byte STR_DATAREQUEST{0x02}; // for type 2 and 3 commands
    static const Byte STR_INDEX{0x02}; // for type 1 commands
    static const Byte STR_BUSY{0x01};

    // Commands in command register (mask 0xf0)
    // TU:   Track update flag
    // MULT: Multple record flag
    static const Byte CMD_RESTORE{0x00};
    static const Byte CMD_SEEK{0x10};
    static const Byte CMD_STEP{0x20};
    static const Byte CMD_STEP_TU{0x30};
    static const Byte CMD_STEPIN{0x40};
    static const Byte CMD_STEPIN_TU{0x50};
    static const Byte CMD_STEPOUT{0x60};
    static const Byte CMD_STEPOUT_TU{0x70};
    static const Byte CMD_READSECTOR{0x80};
    static const Byte CMD_READSECTOR_MULT{0x90};
    static const Byte CMD_WRITESECTOR{0xa0};
    static const Byte CMD_WRITESECTOR_MULT{0xb0};
    static const Byte CMD_READADDRESS{0xc0};
    static const Byte CMD_FORCEIRQ{0xd0};
    static const Byte CMD_READTRACK{0xe0};
    static const Byte CMD_WRITETRACK{0xf0};

    // Internal registers:
    //
    // dr       data register (r/w)
    // tr       track register (r/w)
    // sr       sector register (r/w)
    // cr       command register (w)
    // str      status register (r)
    // stepOffset the step offset of the last STEP_IN/STEP_OUT command
    // side     side of floppy to read/write on, changeable by subclass
    // drq      status of drq pin (= data request)
    // irq      status of irq pin (= interrupt)
    // byteCount    byte counter during read/write
    // strRead  count read access to status register during a read command.
    //          Reset by read from dr.

private:

    Byte                dr, tr, sr, cr, str;
    Byte                stepOffset;
    bool                isDataRequest, isInterrupt;
    bool                side;
    Word                byteCount, strRead;

    // Internal functions
private:

    void                 do_seek(Byte new_track);

public:

    bool isIrq() const
    {
        return isInterrupt;
    }

    bool isDrq() const
    {
        return isDataRequest;
    }

    // Get current side:
    //    false: Side 1
    //    true:  Side 2
    bool getSide() const
    {
        return side;
    }

    Byte getTrack() const
    {
        return tr;
    }

    Byte getSector() const
    {
        return sr;
    }

    Byte getDataRegister() const
    {
        return dr;
    }

    // Set current side:
    //    false: Side 1
    //    true:  Side 2
    void setSide(bool newSide)
    {
        side = newSide;
    }

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

    void setTrack(Byte newTrack)
    {
        tr = newTrack;
    }

    void setSector(Byte newSector)
    {
        sr = newSector;
    }

    void command(Byte command);
    void                     setStatusRecordNotFound();
    bool                     isErrorStatus() const;

    // Read and write functions
    virtual Byte readByte(Word index);
    virtual void writeByte(Word index);
    virtual bool isDriveReady() const;
    virtual bool isWriteProtect();
    virtual bool isRecordNotFound();
    virtual bool isSeekError(Byte new_track);

public:

    void resetIo() override;
    Byte readIo(Word offset) override;
    void writeIo(Word offset, Byte value) override;
    const char *getName() override
    {
        return "wd1793";
    };
    Word sizeOfIo() override
    {
       return 4;
    };

public:

    Wd1793();
    virtual             ~Wd1793();

};

#endif // WD1793_INCLUDED

