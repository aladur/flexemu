/*
    wd1793.h


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
    static const Byte STR_WRITEFAULT{0x20}; // for type 2 and 3 write commands
    static const Byte STR_RECORDNOTFOUND{0x10}; // for type 2 and 3 commands
    static const Byte STR_SEEKERROR{0x10}; // for type 1 commands
    static const Byte STR_CRCERROR{0x08}; // for type 1, 2 and 3 commands
    static const Byte STR_TRACK0{0x04};
    static const Byte STR_LOSTDATA{0x04}; // for type 2 and 3 commands
    static const Byte STR_DATAREQUEST{0x02}; // for type 2 and 3 commands
    static const Byte STR_INDEX{0x02}; // for type 1 commands
    static const Byte STR_BUSY{0x01};

    // Commands in command register (mask 0xf0)
    // TU:   Track update flag
    static const Byte MULTIPLERECORDS{0x10}; // read/write multiple records
    static const Byte CMD_RESTORE{0x00};
    static const Byte CMD_SEEK{0x10};
    static const Byte CMD_STEP{0x20};
    static const Byte CMD_STEP_TU{0x30};
    static const Byte CMD_STEPIN{0x40};
    static const Byte CMD_STEPIN_TU{0x50};
    static const Byte CMD_STEPOUT{0x60};
    static const Byte CMD_STEPOUT_TU{0x70};
    static const Byte CMD_READSECTOR{0x80};
    static const Byte CMD_READSECTOR_MULT{CMD_READSECTOR | MULTIPLERECORDS};
    static const Byte CMD_WRITESECTOR{0xa0};
    static const Byte CMD_WRITESECTOR_MULT{CMD_WRITESECTOR | MULTIPLERECORDS};
    static const Byte CMD_READADDRESS{0xc0};
    static const Byte CMD_FORCEIRQ{0xd0};
    static const Byte CMD_READTRACK{0xe0};
    static const Byte CMD_WRITETRACK{0xf0};

    // Constant bytes used in data stream when writing a track
    // (formatting a disk).
    static const Byte TWO_CRCS{0xF7};
    static const Byte DATA_ADDRESS_MARK{0xFB};
    static const Byte INDEX_MARK{0xFC};
    static const Byte ID_ADDRESS_MARK{0xFE};

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

    Byte dr{0};
    Byte tr{0};
    Byte sr{0};
    Byte cr{0};
    Byte str{0};
    Byte stepOffset{1};
    bool isDataRequest{false};
    bool isInterrupt{false};
    bool side{false};
    Word byteCount{0};
    Word strRead{0};
    Byte indexPulse{0}; // emulate index hole of floppy disc.

    // Internal functions
private:

    void do_seek(Byte new_track);

public:

    bool isIrq()
    {
        indexPulse = (indexPulse + 1) % 16;
        if (!indexPulse)
        {
            // Emulate irq flag on every index pulse.
            setIrq();
        }

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
        // interrupt request to CPU
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
    void setStatusReadError();
    void setStatusWriteError();

    // Read and write functions
    virtual bool startCommand(Byte command_un);
    virtual Byte readByte(Word index, Byte command_un);
    virtual void writeByte(Word &index, Byte command_un);
    virtual bool isDriveReady() const;
    virtual bool isWriteProtect() const;
    virtual bool isRecordNotFound() const;
    virtual bool isSeekError(Byte new_track) const;
    virtual Word getBytesPerSector() const;

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

    Wd1793() = default;
    ~Wd1793() override = default;
};

#endif // WD1793_INCLUDED

