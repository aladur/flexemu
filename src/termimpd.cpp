/*
    termimpd.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024  W. Schwotzer

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


#include "typedefs.h"
#include "termimpd.h"
#include <string>
#include <csignal>

struct sOptions;

DummyTerminalImpl::DummyTerminalImpl(const sOptions & /*p_options*/)
{
}

bool DummyTerminalImpl::init(Word /* [[maybe_unused]] Word reset_key */,
        fct_sigaction fct)
{
    (void)fct;
#ifdef _WIN32
    signal(SIGTERM, fct);
#endif
    return true;
}

void DummyTerminalImpl::reset_serial()
{
}

bool DummyTerminalImpl::has_char_serial()
{
    return false;
}

Byte DummyTerminalImpl::read_char_serial()
{
    return '\0';
}

Byte DummyTerminalImpl::peek_char_serial()
{
    return '\0';
}

void DummyTerminalImpl::write_char_serial(Byte /*value*/)
{
}

bool DummyTerminalImpl::is_terminal_supported()
{
    return false;
}

void DummyTerminalImpl::set_startup_command(
        const std::string & /*startup_command*/)
{
}

void DummyTerminalImpl::reset_terminal_io()
{
}

