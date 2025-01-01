/*
    terminal.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2025  W. Schwotzer

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
#include "termimpi.h"
#include "bobservd.h"
#include <csignal>

class Scheduler;

class TerminalIO : public BObserved
{
    ITerminalImplPtr impl;
    // The Scheduler instance is needed here.
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    Scheduler &scheduler;
    static TerminalIO *instance;
    bool is_initialized{};
#ifdef UNIX
    static bool is_atexit_initialized;
#endif

public:
    bool init(Word reset_key);
    void reset_serial();
    bool has_char_serial();
    Byte read_char_serial();
    Byte peek_char_serial();
    void write_char_serial(Byte val);
    bool is_terminal_supported();
    void set_startup_command(const std::string &startup_command);

    static void on_exit();
#ifdef _WIN32
    static void s_exec_signal(int sig_no);
#endif
#ifdef UNIX
    static void s_exec_signal(int sig_no, siginfo_t *siginfo, void *ptr);
#endif

    TerminalIO() = delete;
    TerminalIO(Scheduler &p_scheduler, ITerminalImplPtr &&p_impl);
    ~TerminalIO() override;

private:
    void reset_terminal_io();
    void exec_signal(int sig_no);
};

#endif // TERMINAL_INCLUDED

