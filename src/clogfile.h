/*
    clogfile.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2003-2018  W. Schwotzer

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

#ifndef _CLOGFILE_H_
#define _CLOGFILE_H_

#include "misc1.h"
#include "mc6809.h"
#include "bcommand.h"

class CSetLogFile : public BCommand
{

public:
    CSetLogFile(Mc6809 &x_cpu, const struct s_cpu_logfile *x_log_file);
    void Execute() override;

protected:
    Mc6809 &cpu;
    const struct s_cpu_logfile *s_log_file;
};

#endif
