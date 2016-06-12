/*
    e2video.h

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2004  W. Schwotzer

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



#ifndef __e2video_h__
#define __e2video_h__

#include <misc1.h>
#include "iodevice.h"

class Memory;
class Inout;


class E2video : public IoDevice {

// Internal registers
protected:
	Memory		*memory;
	Inout		*io;

public:
	Byte		vico1, vico2;
	SWord		divided_block;

public:

	void	resetIo(void);

// public interface
public:

	Byte	    readIo(Word offset);
	void	    writeIo(Word offset, Byte val);
	const char *getName(void) { return "e2video"; };


// Public constructor and destructor
public:
		E2video(Inout *x_io, Memory *x_mem);
		virtual ~E2video();

};

#endif // __e2video_h__

