/*
    termimpi.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2026  W. Schwotzer

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


#ifndef ITERMINALIMPL_INCLUDED
#define ITERMINALIMPL_INCLUDED

#include "typedefs.h"
#include <csignal>
#include <memory>
#include <string>


class TerminalIO;

#ifdef _WIN32
using fct_sigaction = void (*)(int);
#endif
#ifdef UNIX
using fct_sigaction = void (*)(int, siginfo_t *, void *);
#endif

// Polymorphic interface, virtual dtor is required.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class ITerminalImpl
{
    friend class TerminalIO;

public:
    ITerminalImpl() = default;
    virtual ~ITerminalImpl() = default;

    virtual bool init(Word reset_key, fct_sigaction fct) = 0;
    virtual void reset_serial() = 0;
    virtual bool has_char_serial() = 0;
    virtual Byte read_char_serial() = 0;
    virtual Byte peek_char_serial() = 0;
    virtual void write_char_serial(Byte val) = 0;
    virtual bool is_terminal_supported() = 0;
    virtual void set_startup_command(const std::string &startup_command) = 0;

private:
    virtual void reset_terminal_io() = 0;
};

using ITerminalImplPtr = std::unique_ptr<ITerminalImpl>;
#endif
