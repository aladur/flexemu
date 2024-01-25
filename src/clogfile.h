/*
    clogfile.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2003-2024  W. Schwotzer

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

#ifndef CLOGFILE_INCLUDED
#define CLOGFILE_INCLUDED

#include "misc1.h"
#include "mc6809.h"
#include "bcommand.h"
#include "scpulog.h"

class CSetLogFile : public BCommand
{

public:
    CSetLogFile() = delete;
    CSetLogFile(Mc6809 &x_cpu, const s_cpu_logfile &x_log_file);
    CSetLogFile(const CSetLogFile &src) = delete;
    CSetLogFile &operator=(const CSetLogFile &src) = delete;

    void Execute() override;

protected:
    Mc6809 &cpu;
    s_cpu_logfile s_log_file;
};

#endif
