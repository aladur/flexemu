/*
    absgui.cc: abstract graphical user interface


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

#include <sstream>
#include <iomanip>
#include <cassert>
#include "misc1.h"
#include "pia1.h"
#include "absgui.h"
#include "mc6809.h"
#include "memory.h"
#include "inout.h"
#include "schedule.h"
#include "mc6809st.h"
#include "terminal.h"


void AbstractGui::update_cpuview(const Mc6809CpuStatus &stat)
{
    redraw_cpuview(stat);
}

void AbstractGui::redraw_cpuview(const Mc6809CpuStatus &stat)
{
    int i;

    clear_cpuview();
    text(0, 0, "Cycl:");
    text(0, 1, "Inst:");
    text(0, 2, "  PC:");
    text(0, 3, "   S:");
    text(0, 4, "   U:");
    text(0, 5, "   X:");
    text(0, 6, "   Y:");

    text(15, 2, "EFHINZVC");
    text(10, 3, "  CC:");
    text(10, 4, "  DP:");
    text(10, 5, "   A:");
    text(10, 6, "   B:");
    text(19, 5, "  bp1:");
    text(19, 6, "  bp2:");
    text(22, 0, "Freq:");
    text(35, 0, "MHz");

    for (i = 0; i < 6; ++i)
    {
        text(4, i + 8, ":");
    }

    redraw_cpuview_contents(stat);
}

void AbstractGui::redraw_cpuview_contents(const Mc6809CpuStatus &stat)
{
    int i;
    int j;

    int mem_addr = ((stat.s >> 3) << 3) - 16;
    text(6, 2, hexstr(stat.pc));
    text(6, 3, hexstr(stat.s));
    text(6, 4, hexstr(stat.u));
    text(6, 5, hexstr(stat.x));
    text(6, 6, hexstr(stat.y));
    text(15, 3, binstr(stat.cc));
    text(15, 4, hexstr(stat.dp));
    text(15, 5, hexstr(stat.a));
    text(15, 6, hexstr(stat.b));

    std::stringstream cycles_str;
    cycles_str << std::setw(16) << stat.total_cycles;
    text(5, 0, cycles_str.str());

    std::stringstream freq_str;
    freq_str << std::fixed << std::setprecision(2) << std::setw(6) << stat.freq;
    text(28, 0, freq_str.str());

    static const std::string spaces(24, ' ');
    text(6, 1, spaces); // first clear area

    if (stat.mnemonic[0] != '\0')
    {
        text(6, 1, stat.mnemonic);
    }
    else
    {
        text(6, 1, "sorry, no disassembler installed");
    }

    for (i = 0; i < 2; i++)
    {
        if (!cpu.is_bp_set(i))
        {
            text(25, 5 + i, "    ");
        }
        else
        {
            text(25, 5 + i, hexstr((Word)cpu.get_bp(i)));
        }
    }

    int stk = 0;
    Byte ch;

    for (i = 0; i < 6; ++i)
    {
        text(0, i + 8, hexstr(static_cast<Word>(stk + mem_addr)));

        std::string tmp(" ");

        for (j = 0; j < 8; ++j)
        {
            ch = stat.memory[stk + j];
            tmp.append(hexstr(ch));
            tmp.append(" ");
        }

        for (j = 0; j < 8; ++j)
        {
            ch = stat.memory[stk + j];
            tmp.append(ascchr(ch));
        }

        text(5, i + 8, tmp);
        stk += 8;
    }

    redraw_cpuview_impl(stat);
}

void AbstractGui::redraw_cpuview_impl(const Mc6809CpuStatus &)
{
}

void AbstractGui::text(int x, int y, const std::string &str)
{
    assert(x >= 0 && x + str.size() <= CPU_LINE_WIDTH);
    assert(y >= 0 && y < CPU_LINES);
    auto index = CPU_LINE_WIDTH * y + x;

    std::copy(str.begin(), str.cend(), cpustring.begin() + index);
}

void AbstractGui::clear_cpuview()
{
    const std::string delim("\n");

    cpustring.resize(CPU_LINES * CPU_LINE_WIDTH + 1, ' ');
    cpustring[cpustring.size() - 1] = '\0';
    auto size = static_cast<int>(delim.size());

    for (int y = 0; y < CPU_LINES; ++y)
    {
        text(CPU_LINE_WIDTH - size, y, delim);
    }
}

void AbstractGui::output_to_terminal()
{
    if (terminalIO.is_terminal_supported() && inout.is_serpar_address_valid())
    {
        memory.write_byte(inout.serpar_address(), Byte(0xFF));
    }
}

void AbstractGui::output_to_graphic()
{
    if (inout.is_serpar_address_valid())
    {
        memory.write_byte(inout.serpar_address(), 0);
    }
}

void AbstractGui::write_char_serial(Byte /*value*/)
{
}

AbstractGui::AbstractGui(
    Mc6809 &x_cpu,
    Memory &x_memory,
    Inout &x_inout,
    TerminalIO &x_terminalIO)
        : cpu(x_cpu)
        , memory(x_memory)
        , inout(x_inout)
        , terminalIO(x_terminalIO)
{
}

AbstractGui::~AbstractGui()
{
}

