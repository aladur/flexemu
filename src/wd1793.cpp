/*
    wd1793.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2004  W. Schwotzer

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


#include "misc1.h"

#include "wd1793.h"

Wd1793::Wd1793() : dr(0), tr(0), sr(0), cr(0), str(0), isStepIn(0),
    drq(0), irq(0), side(0), byteCount(0), strRead(0)
{
}

Wd1793::~Wd1793()
{
}

void Wd1793::resetIo()
{
    resetIrq();
    isStepIn  = 1;
    drq       = 0;
    side      = 0;
    byteCount = 0;
    strRead   = 0;
    str       = 0;
    cr        = 0;  // clear previous command
    command(0); //execute RESTORE after a reset
}

Byte Wd1793::readIo(Word offset)
{
    static Byte index = 0; // emulate index hole of floppy disc

    switch (offset & 0x03)
    {
        case 0:
            resetIrq();

            if (((cr & 0xe0) == 0x80) && (++strRead == 32))
            {
                drq = 0;
                str &= 0xfc;    // read finished reset drq and busy
            }

            // set index every 16 reads
            if ((str & 0x80))
            {
                return str;
            }

            index = (index + 1) % 16;

            if (!index && !(cr & 0x80))
            {
                return str | 0x02;
            }
            else
            {
                return str;
            }

        case 1:
            return tr;

        case 2:
            return sr;

        case 3:
            if ((cr & 0xf0) == 0xc0)
            {
                switch (byteCount)
                {
                    case 6:
                        dr = tr;
                        break;

                    case 5:
                        dr = side;
                        break;

                    case 4:
                        dr = 1;
                        break;  // sector

                    case 3:
                        dr = 1;
                        break;  // sector length

                    case 2:
                        dr = 0x55;
                        break;

                    case 1:
                        dr = 0x55;
                        break;

                    default:
                        dr = 0;
                } // switch

                byteCount--;
            }
            else
            {
                strRead = 0;

                if (byteCount)
                {
                    dr = readByte(byteCount);
                    byteCount--;
                }
            } // else

            if (!byteCount)
            {
                drq = 0;
                str &= 0xfc;    // read finished reset drq and busy
            }

            return dr;
    }

    return 0;   // default, should never be used!
}   // readIo


void Wd1793::writeIo(Word offset, Byte val)
{
    switch (offset & 0x03)
    {
        case 0:
            resetIrq();
            command(val);
            break;

        case 1:
            tr  = val;
            break;

        case 2:
            sr  = val;
            break;

        case 3:
            dr  = val;

            if (byteCount)
            {
                writeByte(byteCount);
                byteCount--;
            }

            if (!byteCount)
            {
                drq = 0;
                str &= 0xfc;    // write finished reset drq and busy
            }

            break;
    }
}   // writeIo

void Wd1793::do_seek(Byte new_track)
{
    str = 0x20;     // SEEK

    if (seekError(new_track))
    {
        str |= 0x10;
    }
    else
    {
        tr = new_track;
    }

    if (!tr)
    {
        str |= 0x04;
    }

    setIrq();
} // do_seek


void Wd1793::command(Byte command)
{
    Byte        type1;
    static Byte index = 0; // for simulating INDEX bit

    type1 = 0;

    if (!(str & 0x01) || (command & 0xf0) == 0xd0)
    {
        cr = command;
        byteCount = 0;

        switch (cr & 0xf0)
        {
            case 0x00:
                tr = 0;         // RESTORE
                type1 = 1;
                setIrq();
                break;

            case 0x10:
                do_seek(dr);        // SEEK
                type1 = 1;
                break;

            case 0x30:
                if (isStepIn)       // STEP with update
                {
                    do_seek(tr + 1);
                }
                else
                {
                    do_seek(tr - 1);
                }

            case 0x20:
                type1 = 1;      // STEP
                setIrq();
                break;

            case 0x50:
                do_seek(tr + 1);    // STEP IN with update
                isStepIn = 1;
                type1 = 1;
                break;

            case 0x40:
                isStepIn = 1;
                setIrq();
                type1 = 1;
                break;

            case 0x70:
                if (tr)         // STEP OUT with update
                {
                    do_seek(tr - 1);
                }

                isStepIn = 0;
                type1 = 1;
                break;

            case 0x60:
                isStepIn = 0;       // STEP OUT
                setIrq();
                type1 = 1;
                break;

            case 0x80:
                strRead = 0;

                if (recordNotFound())   // READ SECTOR
                {
                    str = 0x10;
                }
                else
                {
                    byteCount = SECTOR_SIZE;
                    drq = 1;
                    str = 0x03;
                }

                break;

            case 0xe0:              // READ TRACK
            case 0x90:
                byteCount = SECTOR_SIZE * 16; // READ SECTOR mult.
                drq = 1;
                str = 0x03;
                break;

            case 0xa0:
                if (writeProtect())   // WRITE SECTOR
                {
                    str = 0x40;
                    break;
                }

                if (recordNotFound())
                {
                    str = 0x10;
                }
                else
                {
                    byteCount = SECTOR_SIZE;
                    drq = 1;
                    str = 0x02;
                }

                break;

            case 0xf0:              // WRITE TRACK
            case 0xb0:
                if (writeProtect())   // WRITE SECTOR mult.
                {
                    str = 0x40;
                    break;
                }

                byteCount = SECTOR_SIZE * 16;
                drq = 1;
                str = 0x03;
                break;

            case 0xc0:
                if (recordNotFound())   // READ ADDRESS
                {
                    str = 0x10;
                }
                else
                {
                    byteCount = 6;
                    drq = 1;
                    str = 0x03;
                }

                break;

            case 0xd0:
                drq = 0;        // FORCE INTERRUPT
                str &= 0xfc;
                byteCount = 0;
                setIrq();
                break;
        } // switch

        if (type1)
        {
            if (driveReady())
            {
                // set index every 16 reads
                index = (index + 1) % 16;
                str = writeProtect() ? 0x64 : 0x24;

                if (!index)
                {
                    str |= 0x02;
                }
            }
            else
            {
                tr  = 1; // ALWAYS SET TRACK TO 1
                // so system info sector never will be found
                str = 0x80;
            }
        } // if
    } // if
}// command


void Wd1793::resetIrq(void)
{
    irq = 0;
}

void Wd1793::setIrq(void)
{
    irq = 1;
    // interrupt request to CPU
}

Byte Wd1793::readByte(Word index)
{
    return (Byte) index;
}

void Wd1793::writeByte(Word index)
{
    (void)index;
}

Byte Wd1793::driveReady(void)
{
    return 1;
}


Byte Wd1793::seekError(Byte)
{
    return 0;
}


Byte Wd1793::recordNotFound(void)
{
    return 0;
}

// should be reimplemented by subclass, return 0x40 if wp, otherwise 0
Byte Wd1793::writeProtect(void)
{
    return 0;
}

