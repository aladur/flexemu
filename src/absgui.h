/*
    absgui.h: abstract gui interface.

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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



#ifndef ABSGUI_INCLUDED
#define ABSGUI_INCLUDED

#include "misc1.h"

#include "e2.h"
#include "flexemu.h"
#include "cpustate.h"
#include <string>
#include <memory>


class Mc6809;
class Inout;
class Memory;
class Mc6809CpuStatus;
class TerminalIO;

class AbstractGui
{
protected:
    Mc6809 &cpu; // Reference to cpu to send interrupts
    Memory &memory; // Reference to memory (incl. video memory access)
    Inout &inout; // Reference to IO-class handling input/output
    TerminalIO &terminalIO; // Reference to terminal data provider.
    std::string cpustring;

protected:
    virtual void redraw_cpuview_impl(const Mc6809CpuStatus &status);
    void clear_cpuview();
    void redraw_cpuview(const Mc6809CpuStatus &status);
    void redraw_cpuview_contents(const Mc6809CpuStatus &status);
    void text(int x, int y, const std::string &str);

public:
    virtual void update_cpuview(const Mc6809CpuStatus &status);
    virtual void output_to_terminal(); // set output to terminal
    virtual void output_to_graphic(); // set output to GUI
    virtual void write_char_serial(Byte value); // write character to printer

public:
    AbstractGui(
        Mc6809 &p_cpu,
        Memory &p_memory,
        Inout &p_inout,
        TerminalIO &p_terminalIO);
    virtual ~AbstractGui() = default;
    AbstractGui(const AbstractGui &src) = delete;
    AbstractGui(AbstractGui &&src) = delete;
    AbstractGui &operator=(const AbstractGui &src) = delete;
    AbstractGui &operator=(AbstractGui &&src) = delete;
};

#endif

