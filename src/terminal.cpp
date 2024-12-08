/*
    terminal.cpp


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


#include "terminal.h"
#include "schedule.h"
#include <iostream>
#include <utility>
#include <string>
#include <cassert>

TerminalIO *TerminalIO::instance = nullptr;
#ifdef UNIX
bool TerminalIO::is_atexit_initialized = false;
#endif

TerminalIO::TerminalIO(Scheduler &p_scheduler, ITerminalImplPtr &&p_impl)
    : impl(std::move(p_impl))
    , scheduler(p_scheduler)
{
    instance = this;
#ifdef UNIX
    if (!is_atexit_initialized)
    {
        // use atexit here to reset the Terminal IO because the X11
        // interface under some error condition like missing DISPLAY
        // variable or X11 protocol error aborts with exit().
        ::atexit(TerminalIO::on_exit);
        is_atexit_initialized = true;
    }
#endif
}

TerminalIO::~TerminalIO()
{
    reset_terminal_io();

    instance = nullptr;
}

void TerminalIO::on_exit()
{
    if (instance != nullptr)
    {
        instance->reset_terminal_io();
    }
}

#ifdef _WIN32
void TerminalIO::s_exec_signal(int sig_no)
{
    if (instance != nullptr)
    {
        instance->exec_signal(sig_no);
    }
}
#endif
#ifdef UNIX
void TerminalIO::s_exec_signal(int sig_no, siginfo_t * /*siginfo*/,
        void * /*ptr*/)
{
    if (instance != nullptr)
    {
        instance->exec_signal(sig_no);
    }
}
#endif

bool TerminalIO::init(Word reset_key)
{
    assert(impl != nullptr);
    if (!impl->init(reset_key, s_exec_signal))
    {
        std::cerr << "unable to initialize terminal\n";
        return false;
    }

    return true;
}

void TerminalIO::reset_serial()
{
    assert(impl != nullptr);
    impl->reset_serial();
}

bool TerminalIO::has_key_serial()
{
    assert(impl != nullptr);
    return impl->has_key_serial();
}

Byte TerminalIO::read_char_serial()
{
    assert(impl != nullptr);
    return impl->read_char_serial();
}

Byte TerminalIO::peek_char_serial()
{
    assert(impl != nullptr);
    return impl->peek_char_serial();
}

void TerminalIO::write_char_serial(Byte value)
{
    assert(impl != nullptr);
    impl->write_char_serial(value);
}

bool TerminalIO::is_terminal_supported()
{
    assert(impl != nullptr);
    return impl->is_terminal_supported();
}

void TerminalIO::set_startup_command(const std::string &startup_command)
{
    assert(impl != nullptr);
    impl->set_startup_command(startup_command);
}

void TerminalIO::reset_terminal_io()
{
    assert(impl != nullptr);
    impl->reset_terminal_io();
}

void TerminalIO::exec_signal(int sig_no)
{
    switch (sig_no)
    {
        case SIGINT:
            Notify(NotifyId::SetNmi);
            break;

#if defined(SIGUSR1)
        case SIGUSR1:
            Notify(NotifyId::SetIrq);
            break;
#endif

#if defined(SIGUSR2)
        case SIGUSR2:
            Notify(NotifyId::SetFirq);
            break;
#endif

#if defined(SIGQUIT)
        // Due to if conditions for SIGQUIT and SIGTERM keep two cases.
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case SIGQUIT:
            scheduler.request_new_state(CpuState::Exit);
            break;
#endif
#if defined(SIGTERM)
        case SIGTERM:
            scheduler.request_new_state(CpuState::Exit);
            break;
#endif

#if defined(SIGTSTP)
        case SIGTSTP:
            scheduler.request_new_state(CpuState::ResetRun);
            break;
#endif
    }
}
