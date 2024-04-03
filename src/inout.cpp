/*
    inout.cpp


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


#include "misc1.h"
#include "inout.h"
#include "e2floppy.h"
#include "mc6821.h"
#include "mc146818.h"
#include "absgui.h"
#include "memory.h"
#include "soptions.h"


Inout::Inout(const struct sOptions &x_options, Memory &x_memory) :
     memory(x_memory)
     , options(x_options)

{
}

void Inout::set_gui(AbstractGui *x_gui)
{
    gui = x_gui;
}

// one second updates are generated by the cpu
// in this method they will be transmitted to all objects
// which need it
void Inout::update_1_second()
{
    if (rtc != nullptr)
    {
        rtc->update_1_second();
    }
}

bool Inout::is_gui_present()
{
    return gui != nullptr;
}

bool Inout::output_to_terminal()
{
#ifdef HAVE_TERMIOS_H
    if (gui != nullptr)
    {
        gui->output_to_terminal();
    }
    return gui != nullptr;
#else
    return false;
#endif // #ifdef HAVE_TERMIOS_H
}

bool Inout::output_to_graphic()
{
    if (gui != nullptr)
    {
        gui->output_to_graphic();
    }
    return gui != nullptr;
}

// Redirect serial character into gui.
// gui provides printer output support.
void Inout::write_char_serial(Byte value)
{
    if (gui != nullptr)
    {
        gui->write_char_serial(value);
    }
}

Word Inout::serpar_address() const
{
    return static_cast<Word>(local_serpar_address);
}

bool Inout::is_serpar_address_valid() const
{
    return local_serpar_address >= 0 && local_serpar_address <= 0xffff;
}

// Set address of SERPAR label. A value < 0 invalidates the address.
void Inout::serpar_address(int value)
{
    local_serpar_address = value;
}

int Inout::read_serpar() const
{
     if (is_serpar_address_valid())
     {
        return memory.read_byte(serpar_address());
     }

     return -1;
}

void Inout::set_rtc(Mc146818 *x_rtc)
{
    rtc = x_rtc;
}

void Inout::UpdateFrom(NotifyId id, void *)
{
    if (id == NotifyId::FirstKeyboardRequest &&
        options.term_mode &&
        is_serpar_address_valid())
    {
        memory.write_byte(serpar_address(), '\xFF');
    }
}

