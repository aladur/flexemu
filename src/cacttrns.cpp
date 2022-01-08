/*
    cacttrns.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2022  W. Schwotzer

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


#include "cacttrns.h"
#include "mc6821.h"


CActiveTransition::CActiveTransition(Mc6821 &x_mc6821,
                                     Mc6821::ControlLine x_control_line) :
    mc6821(x_mc6821), control_line(x_control_line)
{
}

void CActiveTransition::Execute()
{
    mc6821.activeTransition(control_line);
}

