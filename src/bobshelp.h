/*
    bobshelp.h


    Basic helper class used as notification identifier in an observer pattern
    Copyright (C) 2020-2023  W. Schwotzer

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

#ifndef _BOBSHELP_INCLUDED_
#define _BOBSHELP_INCLUDED_

enum class NotifyId : uint8_t
{
    SetIrq,
    SetFirq,
    SetNmi,
    UpdateStatusBar,
    FirstKeyboardRequest,
    RequestScreenUpdate,
    VideoRamBankChanged,
};

#endif // #ifndef _BOBSHELP_INCLUDED_

