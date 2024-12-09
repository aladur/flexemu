/*
    termimpd.h


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


#ifndef DUMMYTERMINALIMPL_INCLUDED
#define DUMMYTERMINALIMPL_INCLUDED

#include "typedefs.h"
#include "termimpi.h"
#include <string>

struct sOptions;
class TerminalIO;

class DummyTerminalImpl : public ITerminalImpl
{
public:
    DummyTerminalImpl() = delete;
    explicit DummyTerminalImpl(const sOptions &p_options);
    ~DummyTerminalImpl() override = default;

    // Interface ITerminalImpl
    bool init(Word reset_key, fct_sigaction fct) override;
    void reset_serial() override;
    bool has_char_serial() override;
    Byte read_char_serial() override;
    Byte peek_char_serial() override;
    void write_char_serial(Byte val) override;
    bool is_terminal_supported() override;
    void set_startup_command(const std::string &startup_command) override;

private:
    void reset_terminal_io() override;
};
#endif
