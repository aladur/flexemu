/*
    wd1793.cpp


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


#include "misc1.h"

#include "wd1793.h"

Wd1793::Wd1793() : dr(0), tr(0), sr(0), cr(0), str(0), stepOffset(1),
    isDataRequest(false), isInterrupt(false), side(false), byteCount(0),
    strRead(0), indexPulse(0)
{
}

Wd1793::~Wd1793()
{
}

void Wd1793::resetIo()
{
    resetIrq();
    stepOffset  = 1;
    isDataRequest = false;
    side      = false;
    byteCount = 0;
    strRead   = 0;
    str       = 0;
    cr        = 0;  // clear previous command
    indexPulse= 0;
    command(0); //execute RESTORE after a reset
}

Byte Wd1793::readIo(Word offset)
{
    switch (offset & 0x03)
    {
        case 0:
            resetIrq();

            if (((cr & 0xe0) == CMD_READSECTOR) && (++strRead == 32))
            {
                isDataRequest = false;
                str &= ~(STR_DATAREQUEST | STR_BUSY); // read finished
            }

            // set index pulse every 16 reads.
            indexPulse = (indexPulse + 1) % 16;
            if (!indexPulse)
            {
                // After max. 16x read from STR drive gets ready
                str &= ~STR_NOTREADY;
            }

            if ((str & STR_NOTREADY))
            {
                return str;
            }

            if (!indexPulse && !(cr & 0x80))
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
            strRead = 0;

            if (byteCount)
            {
                dr = readByte(byteCount, cr & 0xf0);
                if (byteCount != 0)
                {
                    byteCount--;
                }
            }

            if (isDataRequest && !byteCount)
            {
                isDataRequest = false;
                str &= ~(STR_DATAREQUEST | STR_BUSY); // read finished
                setIrq();
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
                writeByte(byteCount, cr & 0xf0);
                if (byteCount != 0)
                {
                    byteCount--;
                }
            }

            if (isDataRequest && !byteCount)
            {
                isDataRequest = false;
                str &= ~(STR_DATAREQUEST | STR_BUSY); // write finished
                setIrq();
            }

            break;
    }
}   // writeIo

void Wd1793::do_seek(Byte new_track)
{
    str = STR_HEADLOADED;     // SEEK

    if (isSeekError(new_track))
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

    if (!(str & STR_BUSY) || (command & 0xf0) == CMD_FORCEIRQ)
    {
        cr = command;
        byteCount = 0;

        switch (cr & 0xf0)
        {
            case CMD_RESTORE:
                do_seek(0);
                tr = 0;
                setIrq();
                break;

            case CMD_SEEK:
                do_seek(dr);
                break;

            case CMD_STEP_TU:
                do_seek(tr + stepOffset);
                FALLTHROUGH;

            case CMD_STEP:
                str = 0;
                setIrq();
                break;

            case CMD_STEPIN_TU:
                stepOffset = 1;
                do_seek(tr + stepOffset);
                break;

            case CMD_STEPIN:
                str = 0;
                stepOffset = 1;
                setIrq();
                break;

            case CMD_STEPOUT_TU:
                stepOffset = static_cast<Byte>(-1);
                do_seek(tr ? tr + stepOffset : tr);
                break;

            case CMD_STEPOUT:
                str = 0;
                stepOffset = static_cast<Byte>(-1);
                setIrq();
                break;

            case CMD_READSECTOR:
                strRead = 0;

                if (isRecordNotFound())
                {
                    str = STR_RECORDNOTFOUND;
                    setIrq();
                }
                else
                {
                    byteCount = getBytesPerSector();
                    isDataRequest = true;
                    str = (STR_BUSY | STR_DATAREQUEST);
                }

                break;

            case CMD_READTRACK:
            case CMD_READSECTOR_MULT:
                byteCount = getBytesPerSector() * 16U;
                isDataRequest = true;
                str = (STR_BUSY | STR_DATAREQUEST);
                break;

            case CMD_WRITESECTOR:
                if (isWriteProtect())
                {
                    str = STR_PROTECTED;
                    setIrq();
                    break;
                }

                if (isRecordNotFound())
                {
                    str = STR_RECORDNOTFOUND;
                    setIrq();
                }
                else
                {
                    byteCount = getBytesPerSector();
                    isDataRequest = true;
                    str = STR_DATAREQUEST;
                }

                break;

            case CMD_WRITETRACK:
            case CMD_WRITESECTOR_MULT:
                if (isWriteProtect())
                {
                    str = STR_PROTECTED;
                    setIrq();
                    break;
                }

                if (startCommand(cr & 0xf0))
                {
                    byteCount = 256;
                    isDataRequest = true;
                    str = (STR_BUSY | STR_DATAREQUEST);
                }
                else
                {
                    str = STR_RECORDNOTFOUND;
                    setIrq();
                }
                break;

            case CMD_READADDRESS:
                if (isDriveReady())
                {
                    byteCount = 6;
                    isDataRequest = true;
                    str = (STR_BUSY | STR_DATAREQUEST);
                }
                else
                {
                    str = STR_RECORDNOTFOUND;
                    setIrq();
                }

                break;

            case CMD_FORCEIRQ:
                isDataRequest = false;
                str &= ~(STR_BUSY | STR_DATAREQUEST);
                byteCount = 0;
                setIrq();
                break;
        } // switch

        if (isType1Command)
        {
            if (isDriveReady())
            {
                // set index pulse every 16 reads.
                indexPulse = (indexPulse + 1) % 16;
                str |= STR_HEADLOADED;
                if(isWriteProtect())
                {
                    str |= STR_PROTECTED;
                }
                if (!indexPulse)
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

// should be reimplemented by subclass.
bool Wd1793::startCommand(Byte)
{
    return true;
}

Byte Wd1793::readByte(Word index, Byte)
{
    return (Byte) index;
}

// should be reimplemented by subclass.
void Wd1793::writeByte(Word &, Byte)
{
}

bool Wd1793::isDriveReady() const
{
    return true;
}


bool Wd1793::isSeekError(Byte) const
{
    return false;
}


bool Wd1793::isRecordNotFound() const
{
    return false;
}

// should be reimplemented by subclass, return 0x40 if wp, otherwise 0
bool Wd1793::isWriteProtect() const
{
    return false;
}

// Set the status register after a read error.
// By trial-and-error (when copying a file) it has been found that flexemu
// responds with the following error messages when an error flag is set.
// The one with the star has been chosen as best choice.
// - STR_RECORDNOTFOUND: DISK FILE READ ERROR.
// - STR_CRCERROR:       DISK FILE READ ERROR.
// * STR_LOSTDATA:       DISK FILE READ ERROR.
void Wd1793::setStatusReadError()
{
    isDataRequest = false;
    str = STR_LOSTDATA;
    byteCount = 0;
}

// Set the status register after a write error.
// By trial-and-error (when copying a file) it has been found that flexemu
// responds with the following error messages when an error flag is set.
// The one with the star has been chosen as best choice.
// - STR_WRITEFAULT:     The error is ignored but the sector is not written.
// - STR_RECORDNOTFOUND: DRIVE NOT READY.
// - STR_CRCERROR:       READ PAST END OF FILE.
// * STR_LOSTDATA:       FILE NOT FOUND.
void Wd1793::setStatusWriteError()
{
    isDataRequest = false;
    str = STR_LOSTDATA;
    byteCount = 0;
}

// should be reimplemented by subclass.
Word Wd1793::getBytesPerSector() const
{
    return 0U;
}

