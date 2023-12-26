/*                                                                              
    asciictl.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2023  W. Schwotzer

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

#ifdef NUL
#undef NUL
#endif
#define NUL '\0'

#ifdef BEL
#undef BEL
#endif
#define BEL '\x07'

#ifdef BS
#undef BS
#endif
#define BS '\x08'

#ifdef HT
#undef HT
#endif
#define HT '\x09'

#ifdef LF
#undef LF
#endif
#define LF '\x0A'

#ifdef VT
#undef VT
#endif
#define VT '\x0B'

#ifdef FF
#undef FF
#endif
#define FF '\x0C'

#ifdef CR
#undef CR
#endif
#define CR '\x0D'

#ifdef SO
#undef SO
#endif
#define SO '\x0E'

#ifdef SI
#undef SI
#endif
#define SI '\x0F'

#ifdef DC1
#undef DC1
#endif
#define DC1 '\x11'

#ifdef DC2
#undef DC2
#endif
#define DC2 '\x12'

#ifdef DC3
#undef DC3
#endif
#define DC3 '\x13'

#ifdef DC4
#undef DC4
#endif
#define DC4 '\x14'

#ifdef CAN
#undef CAN
#endif
#define CAN '\x18'

#ifdef ESC
#undef ESC
#endif
#define ESC '\x1B'

#ifdef DEL
#undef DEL
#endif
#define DEL '\x7F'

#endif
