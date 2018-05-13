/*
    intmem.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

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



#ifndef __intmem_h__
#define __intmem_h__

#include "memory.h"

// memory access within the cpu will be done by inline code
// because of performance reasons

#define READ(x)         memory->read(x)
#define READ_PI(x)      memory->read(x++)
#define READ_WORD(x)            memory->read_word(x)
#define WRITE(x, y)             memory->write(x, y)
#define WRITE_WORD(x, y)        memory->write_word(x, y)

#endif // __intmem_h__

