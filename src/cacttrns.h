/*
    cacttrns.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2003-2024  W. Schwotzer

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

#ifndef CACTTRNS_INCLUDED
#define CACTTRNS_INCLUDED

#include "misc1.h"
#include "mc6821.h"
#include "bcommand.h"


class CActiveTransition : public BCommand
{

public:
    CActiveTransition(Mc6821 &p_mc6821, Mc6821::ControlLine control_line);
    void Execute() override;

protected:
    Mc6821 &mc6821;
    Mc6821::ControlLine control_line;
};

#endif
