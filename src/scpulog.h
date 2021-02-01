/*
    scpulog.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2021  W. Schwotzer

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

#ifndef SCPULOG_INCLUDED
#define SCPULOG_INCLUDED

#include "misc1.h"
#include <string>

struct s_cpu_logfile
{
public:
    s_cpu_logfile() : minAddr(0x0000), maxAddr(0xFFFF),
                      startAddr(0x10000), stopAddr(0x10000)
    {
        logFileName.reserve(PATH_MAX);
    }

    s_cpu_logfile(const s_cpu_logfile &src) :
        minAddr(src.minAddr), maxAddr(src.maxAddr),
        startAddr(src.startAddr), stopAddr(src.stopAddr),
        logFileName(src.logFileName)
    {
    }

    s_cpu_logfile &operator=(const s_cpu_logfile &src)
    {
        minAddr = src.minAddr;
        maxAddr = src.maxAddr;
        startAddr = src.startAddr;
        stopAddr = src.stopAddr;
        logFileName = src.logFileName;
        return *this;
    }

    void reset()
    {
        minAddr = 0x0000;
        maxAddr = 0xFFFF;
        startAddr = 0x10000;
        stopAddr = 0x10000;
        logFileName.clear();
    }

    s_cpu_logfile(s_cpu_logfile &&src) = delete;
    s_cpu_logfile &operator=(s_cpu_logfile &&src) = delete;

    unsigned int minAddr;
    unsigned int maxAddr;
    unsigned int startAddr;
    unsigned int stopAddr;
    std::string logFileName;
};

#endif

