/*
    bprocess.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2004-2018  W. Schwotzer

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
#include <string>

// This class describes a platform independant Process interface

class BProcess
{
public:
    BProcess(const std::string &executable,
             const std::string &directory = "",
             const std::string &argument  = "");
    ~BProcess();
    void AddArgument(const std::string &argument);
    void SetDirectory(const std::string &directory);
    bool Start();     // Start the Process if not started yet
    bool IsRunning(); // Check if Process is running
    const char *GetArguments()  const
    {
        return arguments.c_str();
    };
    const char *GetDirectory()  const
    {
        return directory.c_str();
    };
    const char *GetExecutable() const
    {
        return executable.c_str();
    };

protected:
    std::string executable;
    std::string arguments;
    std::string directory;

#ifdef WIN32
    HANDLE hProcess;
#endif
#ifdef UNIX
    pid_t   pid;
#endif
};

#endif

