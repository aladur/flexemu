/*
    bprocess.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2004  W. Schwotzer

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

#ifndef __bprocess_h__
#define __bprocess_h__

#include "misc1.h"
#include "bstring.h"

// This class describes a platform independant Process interface

class BProcess {
public: 
  BProcess(const char *executable,
		const char *directory = NULL,
		const char *argument  = NULL);
  virtual ~BProcess();
  void AddArgument(const char *argument);
  void SetDirectory(const char *directory);
  bool Start();     // Start the Process if not started yet
  bool IsRunning(); // Check if Process is running
  const char *GetArguments()  const { return arguments;  };
  const char *GetDirectory()  const { return directory;  };
  const char *GetExecutable() const { return executable; };

protected:
  BString executable;
  BString arguments;
  BString directory;

#ifdef WIN32
	HANDLE hProcess;
#endif
#ifdef UNIX
pid_t   pid;
#endif
};

#endif

