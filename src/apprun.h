/*
    apprun.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018  W. Schwotzer

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



#ifndef __apprun_h__
#define __apprun_h__

//#include "misc1.h"
#include "memory.h"
#include "mc6809.h"
#include "da6809.h"
#include "inout.h"
#include "schedule.h"
#include "joystick.h"
#include "mmu.h"
#include "acia1.h"
#include "pia1.h"
#include "pia2.h"
#include "e2floppy.h"
#include "command.h"
#include "e2video.h"
#include "mc146818.h"

struct sGuiOptions;
struct sOptions;


class ApplicationRunner
{
public:
    ApplicationRunner() = delete;
    ApplicationRunner(
            struct sGuiOptions &x_guiOptions,
            struct sOptions &x_options);

    int run();

private:
    struct sGuiOptions &guiOptions;
    struct sOptions &options;
    Memory memory;
    Mc6809 cpu;
    Da6809 disassembler;
    Inout io;
    Scheduler scheduler;
    JoystickIOPtr joystickIO;
    Mmu mmu;
    Acia1 acia1;
    Pia1 pia1;
    Pia2 pia2;
    E2floppy fdc;
    Command command;
    E2video video;
    Mc146818 rtc;
};

#endif
