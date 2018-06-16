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



#ifndef __inout_h__
#define __inout_h__

#include "misc1.h"
#include <stdio.h>
#ifdef HAVE_TERMIOS_H
    #include <termios.h>
#endif
#include "flexemu.h"
#include "flexerr.h"
#include <string>
#include <deque>
#include <mutex>
#include <memory>

#define BACK_SPACE  (0x08)


class E2floppy;
class Mc6809;
class Memory;
class Mc6821;
class Mc146818;
class E2video;
class AbstractGui;
class Scheduler;
class JoystickIO;
class KeyboardIO;
class Pia1;

class Inout
{

    // Internal registers

private:
    std::deque<Byte> key_buffer_serial;
    Mc6809 &cpu;
    Scheduler &scheduler;
#ifdef HAVE_TERMIOS_H
    static      bool   used_serial_io;
    static      struct termios save_termios;
#endif

public:
    static Inout   *instance;
protected:
    AbstractGui    *gui;
    E2floppy       *fdc;
    Mc146818       *rtc;

    // public interface

public:
    void    set_fdc(E2floppy  *x_device);
    void    set_rtc(Mc146818  *x_device);
    AbstractGui *create_gui(JoystickIO &joystickIO,
                            KeyboardIO &keyboardIO, Pia1 &pia1,
                            Memory &memory, E2video &video,
                            struct sGuiOptions &options);
    static void s_exec_signal(int sig_no);

    void    init(Word reset_key);
    void    update_1_second();

    // Communication with GUI
public:
    Word    output_to_terminal();
    Word    output_to_graphic();
    bool    is_gui_present();
    void    main_loop();

protected:
    std::mutex parallel_mutex;
    std::mutex serial_mutex;

    // serial I/O (e.g. terminal)
public:
    void    reset_serial();
    bool    has_key_serial();
    Byte    read_char_serial();
    Byte    peek_char_serial();
    void    write_char_serial(Byte val);
    bool    is_terminal_supported();
    void    signal_reset(int sig_no);

    // Floppy interface
public:
    void    get_drive_status(DiskStatus status[4]);
    std::string get_drive_info(int floppyIndex);

protected:
    int deltaX, deltaY;
    unsigned int buttonMask;
    bool    newValues;
    std::mutex joystick_mutex;

    // Private Interfaces
private:
    static void resetTerminalIO();
    void    initTerminalIO(Word reset_key);
    void    put_char_serial(Byte key);
    void    exec_signal(int sig_no);

    // Public constructor and destructor
public:
    Inout(Mc6809 &x_cpu, Scheduler &x_scheduler);
    ~Inout();

};

#endif // __inout_h__

