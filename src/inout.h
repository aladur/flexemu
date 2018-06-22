/*
    inout.h


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



#ifndef INOUT_INCLUDED
#define INOUT_INCLUDED

#include "misc1.h"
#include "flexemu.h"
#include <string>


class E2floppy;
class Mc6809;
class Memory;
class Mc6821;
class Mc146818;
class VideoControl1;
class VideoControl2;
class AbstractGui;
class Scheduler;
class JoystickIO;
class KeyboardIO;
class TerminalIO;
class Pia1;

class Inout
{
private:
    E2floppy &fdc;
    Mc146818 &rtc;
    AbstractGui *gui;

public:
    AbstractGui *create_gui(
                     JoystickIO &joystickIO
                   , KeyboardIO &keyboardIO
                   , TerminalIO &terminalIO
                   , Pia1 &pia1
                   , Memory &memory
                   , Scheduler &scheduler
                   , Mc6809 &cpu
                   , VideoControl1 &vico1
                   , VideoControl2 &vico2
                   , struct sGuiOptions &options);

    void    update_1_second();

    // Communication with GUI
public:
    Word    output_to_terminal();
    Word    output_to_graphic();
    bool    is_gui_present();
    void    main_loop();

    // Floppy interface
public:
    void    get_drive_status(DiskStatus status[4]);
    std::string get_drive_info(int floppyIndex);

public:
    Inout() = delete;
    Inout(
        E2floppy &x_fdc,
        Mc146818 &x_rtc);
    ~Inout();
};

#endif // INOUT_INCLUDED

