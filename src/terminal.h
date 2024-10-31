/*
    terminal.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2024  W. Schwotzer

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



#ifndef TERMINAL_INCLUDED
#define TERMINAL_INCLUDED

#include "misc1.h"
#ifdef HAVE_TERMIOS_H
    #include <termios.h>
#endif
#include <csignal>
#include "flexemu.h"
#include "flexerr.h"
#include "bobservd.h"
#include "soptions.h"
#include "asciictl.h"
#include <deque>
#include <mutex>


class Scheduler;

class TerminalIO : public BObserved
{
private:
    std::deque<Byte> key_buffer_serial;
    Scheduler &scheduler;
    const struct sOptions &options;
    std::mutex serial_mutex;
#ifdef HAVE_TERMIOS_H
    static bool used_serial_io;
    static struct termios save_termios;
    static bool is_termios_saved;
#endif
    Word init_delay;
    Byte input_delay;
    Byte boot_char;

public:
    static TerminalIO *instance;

#ifdef _WIN32
    static void s_exec_signal(int sig_no);
#else
    static void s_exec_signal(
            int sig_no, siginfo_t * /*info*/, void * /*ucontext*/);
#endif
    void init(Word reset_key);

    void reset_serial();
    bool has_key_serial();
    Byte read_char_serial();
    Byte peek_char_serial();
    void write_char_serial(Byte val);
    bool is_terminal_supported();
    void signal_reset(int sig_no);
    void set_startup_command(const std::string &startup_command);
    void set_boot_char(Byte p_boot_char);

private:
    static void reset_terminal_io();
    void init_terminal_io(Word reset_key);
    void put_char_serial(Byte key);
    void exec_signal(int sig_no);
    void write_char_serial_safe(Byte val);

public:
    TerminalIO() = delete;
    TerminalIO(Scheduler &p_scheduler, const struct sOptions &p_options);
    ~TerminalIO() override = default;
};

#endif // TERMINAL_INCLUDED

