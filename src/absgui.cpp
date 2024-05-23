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
#include <fmt/format.h>


void AbstractGui::update_cpuview(const Mc6809CpuStatus &status)
{
    redraw_cpuview(status);
}

void AbstractGui::redraw_cpuview(const Mc6809CpuStatus &status)
{
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

    for (Word i = 0U; i < CPU_STACK_LINES; ++i)
    {
        text(4, i + 8, ":");
    }

    redraw_cpuview_contents(status);
}

void AbstractGui::redraw_cpuview_contents(const Mc6809CpuStatus &status)
{
    int stack_base = ((status.s / CPU_STACK_BYTES) * CPU_STACK_BYTES) - 16;
    text(6, 2, hexstr(status.pc));
    text(6, 3, hexstr(status.s));
    text(6, 4, hexstr(status.u));
    text(6, 5, hexstr(status.x));
    text(6, 6, hexstr(status.y));
    text(15, 3, binstr(status.cc));
    text(15, 4, hexstr(status.dp));
    text(15, 5, hexstr(status.a));
    text(15, 6, hexstr(status.b));

    std::string cycles_str = fmt::format("{:16}", status.total_cycles);
    text(5, 0, cycles_str);

    std::stringstream freq_str;
    freq_str << std::fixed << std::setprecision(2) << std::setw(6) <<
        status.freq;
    text(28, 0, freq_str.str());

    static const std::string spaces(24, ' ');
    text(6, 1, spaces); // Clear previous mnemonic.

    if (status.mnemonic[0] != '\0')
    {
        text(6, 1, status.mnemonic);
    }
    else
    {
        text(6, 1, "<No disassembler installed>");
    }

    for (int i = 0; i < 2; i++)
    {
        const auto bp = cpu.get_bp(i);
        if (!bp.has_value())
        {
            text(25, 5 + i, "    ");
        }
        else
        {
            text(25, 5 + i, hexstr(bp.value()));
        }
    }

    Word stack_offset = 0U;

    for (Word line = 0U; line < CPU_STACK_LINES; ++line)
    {
        text(0, line + 8, hexstr(static_cast<Word>(stack_base + stack_offset)));

        std::string hex_dump;
        std::string ascii_dump;

        for (Word x = 0U; x < CPU_STACK_BYTES; ++x)
        {
            auto byte = status.memory[stack_offset + x];
            hex_dump.append(hexstr(byte)).append(" ");
            auto ch = static_cast<char>(byte);
            ascii_dump.append(ascchr(ch));
        }

        text(6, line + 8, hex_dump);
        text(6 + 3 * CPU_STACK_BYTES, line + 8, ascii_dump);
        stack_offset += CPU_STACK_BYTES;
    }
    assert(stack_offset == sizeof(status.memory));

    // Mark the current stack position in the hex dump.
    Word base = status.s & 7U;
    text(5 + 3 * base, 10, "[");
    text(8 + 3 * base, 10, "]");

    redraw_cpuview_impl(status);
}

void AbstractGui::redraw_cpuview_impl(const Mc6809CpuStatus &/*status*/)
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
        memory.write_byte(inout.serpar_address(), '\xFF');
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
    Mc6809 &p_cpu,
    Memory &p_memory,
    Inout &p_inout,
    TerminalIO &p_terminalIO)
        : cpu(p_cpu)
        , memory(p_memory)
        , inout(p_inout)
        , terminalIO(p_terminalIO)
{
}

