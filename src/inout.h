/*
    inout.h


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



#ifndef INOUT_INCLUDED
#define INOUT_INCLUDED

#include "misc1.h"
#include "flexemu.h"
#include "bobserv.h"
#include <string>
#include <memory>


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

class Inout : public BObserver
{
private:
    Memory &memory;
    const struct sOptions &options;
    E2floppy *fdc; // fdc not present for Eurocom II/V5
    Mc146818 *rtc; // RTC is an optional device
    std::unique_ptr<AbstractGui> gui;
    int local_serpar_address;

public:
    void create_gui(
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
    void    set_rtc(Mc146818 *x_rtc);

    // Communication with GUI
public:
    bool    output_to_terminal();
    bool    output_to_graphic();
    bool    is_gui_present();
    void    main_loop();

    // Floppy interface
public:
    void set_fdc(E2floppy *x_fdc);
    void    get_drive_status(DiskStatus status[4]);
    std::string get_drive_info(Word drive_nr);

    // local interface
public:
    bool is_serpar_address_valid() const;
    Word serpar_address() const;
    void serpar_address(int value);

    // BObserver interface
public:
    void UpdateFrom(NotifyId id, void *param = nullptr) override;

public:
    Inout(const struct sOptions &x_options, Memory &x_memory);
    ~Inout();
};

#endif // INOUT_INCLUDED

