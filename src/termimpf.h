/*
    termimpf.h


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


#ifndef TERMINALIMPLFACTORY_INCLUDED
#define TERMINALIMPLFACTORY_INCLUDED

#include "flexerr.h"
#include "termimpi.h"
#include "termimpd.h"
#include "termimps.h"
#include <memory>

enum class TerminalType : uint8_t
{
    Dummy,
    Scrolling,
};

struct sOptions;

class TerminalImplFactory
{
public:
    static ITerminalImplPtr Create(TerminalType type, const sOptions &options)
    {
        switch(type)
        {
            case TerminalType::Dummy:
                return std::make_unique<DummyTerminalImpl>(options);

            case TerminalType::Scrolling:
                return std::make_unique<ScrollingTerminalImpl>(options);
        }

        throw FlexException(FERR_INVALID_TERMINAL_TYPE, static_cast<int>(type));
    }

    static TerminalType GetType(int value)
    {
        using T = std::underlying_type_t<TerminalType>;

        switch (value)
        {
            case static_cast<T>(TerminalType::Scrolling):
                 return TerminalType::Scrolling;

            default:
                 throw FlexException(FERR_INVALID_TERMINAL_TYPE, value);
        }
    }
};
#endif
