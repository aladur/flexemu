/*
    command.h

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



#ifndef COMMAND_INCLUDED
#define COMMAND_INCLUDED

#include "misc1.h"
#include "iodevice.h"
#include "bobservd.h"
#include "asciictl.h"
#include <string>

enum {
MAX_COMMAND = 128,
};

enum {
INVALID_DRIVE = 4, /* Identifier for an invalid drive */
};

class Inout;
class E2floppy;
class Scheduler;


class Command : public IoDevice, public BObserved
{

    // Internal registers
protected:

    Inout       &inout;
    Scheduler   &scheduler;
    E2floppy    &fdc;
    char         command[MAX_COMMAND];
    Word         command_index;
    Word         answer_index;
    std::string  answer;

    // private interface:
private:
    void        skip_token(char **);
    const char  *next_token(char **, int *);

    // public interface
public:

    void resetIo() override;
    Byte readIo(Word offset) override;
    void writeIo(Word offset, Byte val) override;
    const char *getName() override
    {
        return "command";
    };
    Word sizeOfIo() override
    {
        return 1;
    }

public:
    Command(
            Inout &x_inout,
            Scheduler &x_scheduler,
            E2floppy &x_fdc);
    ~Command() override;
};

#endif // COMMAND_INCLUDED

