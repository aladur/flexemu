/*
    asciictl.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2023-2026  W. Schwotzer

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


#ifndef ASCIICTL_INCLUDED
#define ASCIICTL_INCLUDED

#include <cstdint>


// Use ASCII control identifiers as globally defined, for example see:
// https://en.wikipedia.org/wiki/Control_character
// NOLINTBEGIN(misc-confusable-identifiers)
enum : uint8_t {
NUL = '\x00',
SOH = '\x01',
STX = '\x02',
ETX = '\x03',
EOT = '\x04',
ENQ = '\x05',
ACK = '\x06',
BEL = '\x07',
BS = '\x08',
HT = '\x09',
LF = '\x0A',
VT = '\x0B',
FF = '\x0C',
CR = '\x0D',
SO = '\x0E',
SI = '\x0F',
DLE = '\x10',
DC1 = '\x11',
DC2 = '\x12',
DC3 = '\x13',
DC4 = '\x14',
NAK = '\x15',
SYN = '\x16',
ETB = '\x17',
CAN = '\x18',
EM = '\x19',
SUB = '\x1A',
ESC = '\x1B',
FS = '\x1C',
GS = '\x1D',
RS = '\x1E',
US = '\x1F',
DEL = '\x7F',
};
// NOLINTEND(misc-confusable-identifiers)

#endif
