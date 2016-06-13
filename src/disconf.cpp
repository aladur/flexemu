/*
    disconf.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2003-2004 W. Schwotzer

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
#include "disconf.h"
#include "bident.h"
#include "buint.h"


DisassemblerConfig::DisassemblerConfig()
{
}

DisassemblerConfig::~DisassemblerConfig()
{
    Clear();
}

void DisassemblerConfig::Clear()
{
    entryPoints.SetEmpty();
    labelList.SetEmpty();
    equateList.SetEmpty();
    dataRangesInt8.Clear();
    dataRangesInt16.Clear();
    dataRangesInt32.Clear();
    dataRangesChar.Clear();
}

void DisassemblerConfig::SetStartAddress(DWord address, const char *pLabel)
{
    BIdentifier i;

    i.SetTo(address, pLabel);
    entryPoints.Add(address);
    labelList.Add(i);
}

bool DisassemblerConfig::Contains(const BInterval &i, int type) const
{
    return (((type & DISCONF_CODE) && codeRanges.Contains(i)) ||
            ((type & DISCONF_INT8)  && dataRangesInt8.Contains(i))  ||
            ((type & DISCONF_INT16) && dataRangesInt16.Contains(i)) ||
            ((type & DISCONF_INT32) && dataRangesInt32.Contains(i)) ||
            ((type & DISCONF_CHAR)  && dataRangesChar.Contains(i)));
}

bool DisassemblerConfig::Contains(DWord address, int type) const
{
    return (((type & DISCONF_CODE) && codeRanges.Contains(address)) ||
            ((type & DISCONF_INT8)  && dataRangesInt8.Contains(address))  ||
            ((type & DISCONF_INT16) && dataRangesInt16.Contains(address)) ||
            ((type & DISCONF_INT32) && dataRangesInt32.Contains(address)) ||
            ((type & DISCONF_CHAR)  && dataRangesChar.Contains(address)));
}

void DisassemblerConfig::PrintOn(FILE *fp) const
{
    // unfinished
    fprintf(fp, "code ranges:\n");
    codeRanges.PrintOn(fp);
    fprintf(fp, "data1 ranges:\n");
    dataRangesInt8.PrintOn(fp);
    fprintf(fp, "data2 ranges:\n");
    dataRangesInt16.PrintOn(fp);
    fprintf(fp, "data4 ranges:\n");
    dataRangesInt32.PrintOn(fp);
    fprintf(fp, "data(char) ranges:\n");
    dataRangesChar.PrintOn(fp);
}

void DisassemblerConfig::Add(const BAddressRanges &r,
                             int type /* = DISCONF_CODE */)
{
    if (type == DISCONF_CODE)
    {
        codeRanges.Add(r);
    }

    if (type == DISCONF_CHAR)
    {
        dataRangesChar.Add(r);
    }

    if (type == DISCONF_INT8)
    {
        dataRangesInt8.Add(r);
    }

    if (type == DISCONF_INT16)
    {
        dataRangesInt16.Add(r);
    }

    if (type == DISCONF_INT32)
    {
        dataRangesInt32.Add(r);
    }
}

void DisassemblerConfig::Add(const BInterval &i, int type /* = DISCONF_INT8 */)
{
    switch (type)
    {
        case DISCONF_CODE:
            codeRanges.Add(i);
            dataRangesInt8.Remove(i);
            dataRangesInt16.Remove(i);
            dataRangesInt32.Remove(i);
            dataRangesChar.Remove(i);
            break;

        case DISCONF_INT8:
            codeRanges.Add(i);
            codeRanges.Remove(i);
            dataRangesInt16.Remove(i);
            dataRangesInt32.Remove(i);
            dataRangesChar.Remove(i);
            break;

        case DISCONF_INT16:
            codeRanges.Add(i);
            codeRanges.Remove(i);
            dataRangesInt8.Remove(i);
            dataRangesInt32.Remove(i);
            dataRangesChar.Remove(i);
            break;

        case DISCONF_INT32:
            dataRangesInt32.Add(i);
            codeRanges.Remove(i);
            dataRangesInt8.Remove(i);
            dataRangesInt16.Remove(i);
            dataRangesChar.Remove(i);
            break;

        case DISCONF_CHAR:
            dataRangesChar.Add(i);
            codeRanges.Remove(i);
            dataRangesInt8.Remove(i);
            dataRangesInt16.Remove(i);
            dataRangesInt32.Remove(i);
            break;
    }
}

void DisassemblerConfig::Remove(const BInterval &i)
{
    codeRanges.Remove(i);
    dataRangesChar.Remove(i);
    dataRangesInt8.Remove(i);
    dataRangesInt16.Remove(i);
    dataRangesInt32.Remove(i);
}

bool DisassemblerConfig::GetFirstEntryPoint(DWord *result) const
{
    BUInt first;

    first = entryPoints.GetFirst();

    if (!first)
    {
        return false;
    }

    *result = (DWord)first;
    return true;
}

bool DisassemblerConfig::GetNextEntryPoint(DWord *result) const
{
    BUInt next;

    next = entryPoints.GetNext();

    if (!next)
    {
        return false;
    }

    *result = (DWord)next;
    return true;
}
