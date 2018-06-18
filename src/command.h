/*
    command.h

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



#ifndef __command_h__
#define __command_h__

#include "misc1.h"

#include "iodevice.h"

#define MAX_COMMAND     (128)

#define UNKNOWN         (0)
#define PARAM           (1)
#define PATH            (2)
#define UNABLE_MOUNT        (3)
#define UNABLE_UMOUNT       (4)
#define UNABLE_UPDATE       (5)
#define CANT_CHANGE_GRAPHIC (6)
#define UNABLE_FORMAT       (7)
#define MEMORY_ERROR        (8)
#define MAX_ERR         (MEMORY_ERROR+1)

#define INVALID_DRIVE       (4)

#define CR          (0x0d)

#define ANSWER_ERR(type) {\
        answer = new char[strlen(err[type]) + 1]; \
        strcpy(answer, err[type]); \
    }

class Mc6809;
class Inout;
class E2floppy;
class Scheduler;


class Command : public IoDevice
{

    // Internal registers
protected:

    Mc6809      &cpu;
    Inout       &inout;
    Scheduler   &scheduler;
    E2floppy    &fdc;
    char         command[MAX_COMMAND];
    Word         command_index;
    Word         answer_index;
    char        *answer;
    const char  *err[MAX_ERR];

    // private interface:
private:
    void        skip_token(char **);
    const char  *next_token(char **, int *);
    const char  *modify_command_token(char *p);

    // public interface
public:

    void    resetIo();
    Byte    readIo(Word offset);
    void    writeIo(Word offset, Byte val);
    virtual const char *getName()
    {
        return "command";
    };
    virtual int sizeOfIo()
    {
        return 1;
    }

public:
    Command(
            Inout &x_inout,
            Mc6809 &x_cpu,
            Scheduler &x_scheduler,
            E2floppy &x_fdc);
    virtual ~Command();
};

#endif // __command_h__

