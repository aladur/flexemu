/*
    inout.h


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



#ifndef __inout_h__
#define __inout_h__

#include <misc1.h>
#include <stdio.h>
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#include "flexemu.h"
#include "bstring.h"

#define KEY_BUFFER_SIZE (8)
#define BELL		(0x07)
#define BACK_SPACE	(0x08)

#define L_MB		(4)
#define M_MB		(2)
#define R_MB		(1)
#define SHIFT_KEY	(8)
#define CONTROL_KEY	(16)

class BMutex;
class E2floppy;
class Mc6809;
class Memory;
class Mc6821;
class Mc146818;
class E2video;
class AbstractGui;
class Scheduler;

class Inout {

// Internal registers

private:
	Byte	  	key_buffer[KEY_BUFFER_SIZE];
	Word	  	in, out;
	Byte	  	key_buffer_ser[KEY_BUFFER_SIZE];
	Word	  	in_ser, out_ser;
	Mc6809	       *cpu;
	struct sGuiOptions *options;
#ifdef HAVE_TERMIOS_H
	static		bool   used_serial_io;
	static		struct termios save_termios;
#endif

public:
	AbstractGui    *gui;
	static Inout   *instance;
protected:
	E2floppy       *fdc;
	Memory         *memory;
	Mc146818       *rtc;
	Mc6821         *pia1, *pia2;
	E2video        *video;
	Scheduler      *schedy;
	
// public interface

public:
	void	set_gui(AbstractGui *x_gui);
	void	set_fdc(E2floppy  *x_device);
	void	set_rtc(Mc146818  *x_device);
	void	set_memory(Memory  *x_memory);
	void	set_pia1(Mc6821 *x_device);
	void	set_pia2(Mc6821 *x_device);
	void	set_video(E2video *x_video);
	void	set_scheduler(Scheduler *x_sched);
	AbstractGui *create_gui(int type);
	static void s_exec_signal(int sig_no);

	void	reset(void);
	void	init(Word reset_key);
	void	update_1_second(void);
	void	set_bell(Word x_percent);

// Communication with GUI
public:
	Word	output_to_terminal(void);
	Word	output_to_graphic(void);

// parallel I/O
public:
	void	reset_parallel(void);
	bool	poll(void);
	Byte	read_ch(void);
	Byte	read_queued_ch(void);
	void	put_ch(Byte key);
	bool	is_terminal_supported(void);
protected:
	bool	key_buffer_full(void);
	BMutex	*pmutex;
	bool	is_parallel_ch_in_queue;

// serial I/O
public:
	void	reset_serial(void);
	bool	poll_serial(void);
	Byte	read_ch_serial(void);
	Byte	read_queued_ch_serial(void);
	void	write_ch_serial(Byte val);
	void	signal_reset(int sig_no);

// Floppy interface
public:
	void    get_drive_status(tDiskStatus status[4]);
	BString get_drive_info(int floppyIndex);

// Joystick Interface
public:
	void	reset_joystick(void);
	bool	get_joystick(int *deltaX, int *deltaY, unsigned int *buttonMask);
	void	put_joystick(int deltaX, int deltaY);
	void	put_joystick(unsigned int buttonMask);
protected:
	int	deltaX, deltaY;
	unsigned int buttonMask;
	bool	newValues;
	BMutex	*jmutex;

// Private Interfaces
private:
	static void resetTerminalIO();
	void	initTerminalIO(Word reset_key);
	void	put_ch_serial(Byte key);
	Byte	key_buffer_full_serial(void);
	void	exec_signal(int sig_no);

// Public constructor and destructor
public:
		Inout(Mc6809 *x_cpu, struct sGuiOptions *poptions);
		~Inout();

};

#endif // __inout_h__

