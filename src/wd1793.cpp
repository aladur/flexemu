/*
    wd1793.cpp


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


#include "misc1.h"

#include "wd1793.h"

Wd1793::Wd1793() : dr(0), tr(0), sr(0), cr(0), str(0), isStepIn(false),
    isDataRequest(false), isInterrupt(false), side(0), byteCount(0), strRead(0)
{
}

Wd1793::~Wd1793()
{
}

void Wd1793::resetIo()
{
    resetIrq();
    isStepIn  = true;
    isDataRequest = false;
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
                isDataRequest = false;
                str &= ~(STR_DATAREQUEST | STR_BUSY); // read finished
            }

            // set index every 16 reads
            if ((str & STR_NOTREADY))
            {
                return str;
            }

            index = (index + 1) % 16;

            if (!index && !(cr & 0x80))
            {
                return str | STR_DATAREQUEST;
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
                    if (!isErrorStatus())
                    {
                        byteCount--;
                    }
                }
            } // else

            if (!byteCount)
            {
                isDataRequest = false;
                str &= ~(STR_DATAREQUEST | STR_BUSY); // read finished
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
                if (!isErrorStatus())
                {
                    byteCount--;
                }
            }

            if (!byteCount)
            {
                isDataRequest = false;
                str &= ~(STR_DATAREQUEST | STR_BUSY); // write finished
            }

            break;
    }
}   // writeIo

void Wd1793::do_seek(Byte new_track)
{
    str = STR_HEADLOADED;     // SEEK

    if (seekError(new_track))
    {
        str |= STR_SEEKERROR;
    }
    else
    {
        tr = new_track;
    }

    if (!tr)
    {
        str |= STR_TRACK0;
    }

    setIrq();
} // do_seek


void Wd1793::command(Byte command)
{
    bool isType1Command = !(command & 0x80);
    static Byte index = 0; // for simulating INDEX bit

    if (!(str & STR_BUSY) || (command & 0xf0) == 0xd0)
    {
        cr = command;
        byteCount = 0;

        switch (cr & 0xf0)
        {
            case 0x00:
                tr = 0;         // RESTORE
                setIrq();
                break;

            case 0x10:
                do_seek(dr);        // SEEK
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
                setIrq();       // STEP
                break;

            case 0x50:
                do_seek(tr + 1);    // STEP IN with update
                isStepIn = true;
                break;

            case 0x40:
                isStepIn = true;
                setIrq();
                break;

            case 0x70:
                if (tr)         // STEP OUT with update
                {
                    do_seek(tr - 1);
                }

                isStepIn = false;
                break;

            case 0x60:
                isStepIn = false;       // STEP OUT
                setIrq();
                break;

            case 0x80:
                strRead = 0;

                if (recordNotFound())   // READ SECTOR
                {
                    str = STR_SEEKERROR;
                }
                else
                {
                    byteCount = SECTOR_SIZE;
                    isDataRequest = true;
                    str = (STR_BUSY | STR_DATAREQUEST);
                }

                break;

            case 0xe0:              // READ TRACK
            case 0x90:
                byteCount = SECTOR_SIZE * 16; // READ SECTOR mult.
                isDataRequest = true;
                str = (STR_BUSY | STR_DATAREQUEST);
                break;

            case 0xa0:
                if (writeProtect())   // WRITE SECTOR
                {
                    str = STR_PROTECTED;
                    break;
                }

                if (recordNotFound())
                {
                    str = STR_SEEKERROR;
                }
                else
                {
                    byteCount = SECTOR_SIZE;
                    isDataRequest = true;
                    str = STR_DATAREQUEST;
                }

                break;

            case 0xf0:              // WRITE TRACK
            case 0xb0:
                if (writeProtect())   // WRITE SECTOR mult.
                {
                    str = STR_PROTECTED;
                    break;
                }

                byteCount = SECTOR_SIZE * 16;
                isDataRequest = true;
                str = (STR_BUSY | STR_DATAREQUEST);
                break;

            case 0xc0:
                if (recordNotFound())   // READ ADDRESS
                {
                    str = STR_SEEKERROR;
                }
                else
                {
                    byteCount = 6;
                    isDataRequest = true;
                    str = (STR_BUSY | STR_DATAREQUEST);
                }

                break;

            case 0xd0:
                isDataRequest = false; // FORCE INTERRUPT
                str &= ~(STR_BUSY | STR_DATAREQUEST);
                byteCount = 0;
                setIrq();
                break;
        } // switch

        if (isType1Command)
        {
            if (driveReady())
            {
                // set index every 16 reads
                index = (index + 1) % 16;
                str = (STR_HEADLOADED | STR_TRACK0);
                if(writeProtect())
                {
                    str |= STR_PROTECTED;
                }
                if (!index)
                {
                    str |= STR_INDEX;
                }
            }
            else
            {
                tr  = 1; // ALWAYS SET TRACK TO 1
                // so system info sector never will be found
                str = STR_NOTREADY;
            }
        } // if
    } // if
}// command

Byte Wd1793::readByte(Word index)
{
    return (Byte) index;
}

void Wd1793::writeByte(Word index)
{
    (void)index;
}

bool Wd1793::driveReady()
{
    return true;
}


bool Wd1793::seekError(Byte)
{
    return false;
}


bool Wd1793::recordNotFound()
{
    return false;
}

// should be reimplemented by subclass, return 0x40 if wp, otherwise 0
bool Wd1793::writeProtect()
{
    return false;
}

void Wd1793::setStatusRecordNotFound()
{
    isDataRequest = 0;
    str = STR_RECORDNOTFOUND;
    byteCount = 0;
}

bool Wd1793::isErrorStatus()
{
    return (str & (STR_RECORDNOTFOUND | STR_SEEKERROR)) != 0;
}

