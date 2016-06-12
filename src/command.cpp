/*
    command.cpp


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


#include <misc1.h>
#ifdef ultrix
#include <strings.h>
#endif
#include <ctype.h>
#include <new>

#include "e2.h"
#include "command.h"
#include "absgui.h"
//#include "filecnts.h"
#include "bstring.h"
#include "flexerr.h"
#include "e2floppy.h"
#include "inout.h"
#include "schedule.h"


Command::Command(Inout *x_io, Mc6809 *x_cpu, Scheduler* x_sched) :
	cpu(x_cpu), io(x_io), schedy(x_sched), fdc(NULL), answer(NULL)
{
	err[UNKNOWN]		= "Unknown command";
	err[PARAM]		= "Parameter invalid";
	err[PATH]		= "Nonexistent path";
	err[UNABLE_MOUNT]  	= "Unable to mount drive";
	err[UNABLE_UMOUNT] 	= "Unable to umount drive";
	err[UNABLE_UPDATE] 	= "Unable to update drive. There are open files";
	err[CANT_CHANGE_GRAPHIC]= "Unable to change to graphic mode";
	err[UNABLE_FORMAT] 	= "Unable to format disk";
	err[MEMORY_ERROR] 	= "Not enough memory to execute";
} 

Command::~Command()
{
	delete [] answer;
}

void Command::set_fdc(E2floppy *device)
{
	fdc = device;
}


void Command::resetIo(void)
{
	command_index = 0;
	answer_index  = 0;
	answer        = NULL;
}

Byte Command::readIo(Word offset)
{
	char *pch;

	if (answer != NULL) {
		pch = answer + answer_index++;
		if (*pch == '\0') {
			if (answer != gMemoryAllocationErrorString)
				// unfinished: Debug assersion on Win32 after "emu freq"
				delete [] answer;
			answer = NULL;
			answer_index = 0;
			return 0x00;
		} else
			if (*pch == '\n')
				return CR;
			else
				return *pch;
	}
	return 0x00; 
}

char *Command::next_token(char *p, int *pcount)
{
	while (p != NULL && *p != '\0' && *p == ' ')
		p++;
	if (p != NULL && *p != '\0')
		(*pcount)++;
	return p;
}


char *Command::skip_token(char *p)
{
	while (p != NULL && *p != '\0' && *p != ' ')
		p++;
	if (p != NULL && *p == ' ')
		*(p++) = '\0';
	return p;
}

// remove drive id and file extension
// from command token
char *Command::modify_command_token(char *p)
{
	char *p1 = p;
	int i = 0;

	if (p1 == NULL)
		return NULL;
	if (*p1 == '\0')
		return p1;
	if (isdigit(*p1) && *(p1+1) == '.')
		p1 += 2;
	do {
		if (*(p1+i) == '.')
		{
			*(p1+i) = '\0';
			break;
		}
		i++;
	} while (*(p1+i) != '\0');
	return p1;
}

void Command::writeIo(Word offset, Byte val)
{
	try {
		if (answer != gMemoryAllocationErrorString)
			delete [] answer;
		answer = NULL;
		if (command_index < MAX_COMMAND - 1)
			command[command_index++] = val;
		else
			command[command_index] = val;
		if (val == '\0') {
			char *p, *arg1, *arg2, *arg3, *arg4;
			int   count, number;

			command_index = 0;
			answer_index  = 0;
			number	      = INVALID_DRIVE;
			count         = 0;
			arg1 = arg2 = arg3 = arg4 = "";
			p = arg1 = next_token(command, &count);	// get arg1
			p = skip_token(p);
			p = arg2 = next_token(p, &count);	// get arg2
			p = skip_token(p);
			p = arg3 = next_token(p, &count);	// get arg3
			p = skip_token(p);
			p = arg4 = next_token(p, &count);	// get arg4
			p = skip_token(p);
			p = next_token(p, &count);

			arg1 = modify_command_token(arg1);

			switch (count) {
			case 1:
				if (stricmp(arg1, "exit") == 0 && schedy != NULL) {
					schedy->set_new_state(S_EXIT);
					return;
				} else if (stricmp(arg1, "irq")  == 0 && cpu != NULL) {
					cpu->set_irq();
					return;
				} else if (stricmp(arg1, "firq")  == 0 && cpu != NULL) {
					cpu->set_firq();
					return;
				} else if (stricmp(arg1, "nmi")  == 0 && cpu != NULL) {
					cpu->set_nmi();
					return;
				} else if (stricmp(arg1, "terminal") == 0) {
					io->output_to_terminal();
					return;
				} else if (stricmp(arg1, "graphic") == 0) {
					if (!io->output_to_graphic())
						ANSWER_ERR(CANT_CHANGE_GRAPHIC);
					return;
				} else if (stricmp(arg1, "freq") == 0) {
					answer = new char[16];
					sprintf(answer, "%.2f MHz",
					   schedy->get_frequency());
					answer_index = 0;
					return;
				} else if (stricmp(arg1, "cycles") == 0) {
					answer = new char[24];
#ifdef __GNUC__
					sprintf(answer, "%llu cycles",
					   schedy->get_total_cycles());
#endif
#ifdef _MSCVER
					sprintf(answer, "%I64u cycles",
					   schedy->get_total_cycles());
#endif
					answer_index = 0;
					return;
				} else if (stricmp(arg1, "xt") == 0) {
					io->create_gui(GUI_XTOOLKIT);
					return;
				} else if (stricmp(arg1, "windows") == 0) {
					io->create_gui(GUI_WINDOWS);
					return;
				} else if (stricmp(arg1, "info") == 0 && fdc != NULL) {
					BString str;

					count =  0;
					str = "";
					for (number = 0; number <= 3; number++) {
						str += fdc->drive_info(number);
					}
					answer = new char[str.length() + 1];
					strcpy(answer, str);
					answer_index = 0;
					return;
				} else if (stricmp(arg1, "update") == 0 &&
				  fdc != NULL) {
					if (!fdc->update_all_drives())
						ANSWER_ERR(UNABLE_UPDATE);
					return;
				}
				break;
			case 2:
				if (stricmp(arg1, "freq") == 0) {
					float freq;

					if ((sscanf(arg2, "%f", &freq) == 1) &&
						freq >= 0.0)
						schedy->set_frequency(freq);
					return;
				}
				if ((sscanf(arg2, "%d", &number) != 1) ||
				   number < 0 || number > 3) {
					ANSWER_ERR(PARAM);
					return;
				}
				if (stricmp(arg1, "umount") == 0 && fdc != NULL) {
					if (!fdc->umount_drive(number))
						ANSWER_ERR(UNABLE_UMOUNT);
					return;
				} else if (stricmp(arg1, "info") == 0 && fdc != NULL) {
					BString str;

					str = fdc->drive_info(number);
					answer = new char[str.length() + 1];
					strcpy(answer, str);
					answer_index = 0;
					return;
				} else if (stricmp(arg1, "update") == 0 &&
				  fdc != NULL) {
					if (!fdc->update_drive(number))
						ANSWER_ERR(UNABLE_UPDATE);
					return;
				}
				break;
			case 3:	if (stricmp(arg1, "mount") == 0 && fdc != NULL) {
					if ((sscanf(arg3, "%d", &number) != 1) ||
			   			number < 0 || number > 3) {
							ANSWER_ERR(PARAM);
							return;
						}
					if (!fdc->mount_drive(arg2, number))
						ANSWER_ERR(UNABLE_MOUNT);
					return;
				} else if (stricmp(arg1, "rmount") == 0 && fdc != NULL) {
					if ((sscanf(arg3, "%d", &number) != 1) ||
			   			number < 0 || number > 3) {
							ANSWER_ERR(PARAM);
							return;
						}
					if (!fdc->mount_drive(arg2, number, MOUNT_RAM))
						ANSWER_ERR(UNABLE_MOUNT);
					return;
				}
				break;
			case 4: if (stricmp(arg1, "format") == 0 && fdc!=NULL) {
					int trk, sec;

					if ((sscanf(arg3, "%d", &trk) != 1) ||
						(sscanf(arg4, "%d", &sec) != 1) ||
			   			trk < 2 || sec < 5) {
							ANSWER_ERR(PARAM);
							return;
						}
					if (!fdc->format_disk(trk, sec, arg2, TYPE_DSK_CONTAINER))
						ANSWER_ERR(UNABLE_FORMAT);
					return;
				}
				break;
			} // switch
			ANSWER_ERR(UNKNOWN);
		} // if
	} catch (std::bad_alloc UNUSED(&e)) {
		answer = gMemoryAllocationErrorString;
		answer_index = 0;
	}
}

