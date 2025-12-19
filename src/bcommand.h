/*
    bcommand.h


    Basic class providing base class for a command pattern
    (See the book Go4)

    Copyright (C) 2003-2025  W. Schwotzer

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

#ifndef BCOMMAND_INCLUDED
#define BCOMMAND_INCLUDED

#include <memory>

class BCommand
{

public:
    BCommand(const BCommand &src) = default;
    BCommand &operator=(const BCommand &src) = default;
    BCommand(BCommand &&src) = delete;
    BCommand &operator=(BCommand &&src) = delete;
    virtual ~BCommand() = default;

    virtual void Execute() = 0;

protected:
    BCommand() = default;
};

using BCommandSPtr = std::shared_ptr<BCommand>;

#endif
