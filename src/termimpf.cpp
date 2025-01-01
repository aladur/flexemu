/*
    termimpf.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2025  W. Schwotzer

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


#include "flexerr.h"
#include "termimpf.h"
#include "termimpi.h"
#include "termimpc.h"
#include "termimpd.h"
#include "termimps.h"
#include <memory>


ITerminalImplPtr TerminalImplFactory::Create(TerminalType type,
        const sOptions &options)
{
    switch(type)
    {
        case TerminalType::Dummy:
            return std::make_unique<DummyTerminalImpl>(options);

        case TerminalType::Scrolling:
            return std::make_unique<ScrollingTerminalImpl>(options);

        case TerminalType::NCurses:
            return std::make_unique<NCursesTerminalImpl>(options);
    }

    throw FlexException(FERR_INVALID_TERMINAL_TYPE, static_cast<int>(type));
}

TerminalType TerminalImplFactory::GetType(int value)
{
    using T = std::underlying_type_t<TerminalType>;

    switch (value)
    {
        case static_cast<T>(TerminalType::Scrolling):
             return TerminalType::Scrolling;

        case static_cast<T>(TerminalType::NCurses):
             return TerminalType::NCurses;

        default:
             throw FlexException(FERR_INVALID_TERMINAL_TYPE, value);
    }
}
