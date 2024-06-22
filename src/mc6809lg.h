/*
    mc6809lg.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024  W. Schwotzer

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



#ifndef CPULOGGER_INCLUDED
#define CPULOGGER_INCLUDED

#include "scpulog.h"
#include <fstream>
#include <string>


class CpuStatus;

class Mc6809Logger
{
public:
    Mc6809Logger() = default;
    virtual ~Mc6809Logger() = default;

    bool doLogging(Word pc) const;
    void logCurrentState(const CpuStatus &cpuState);
    bool setLoggerConfig(const Mc6809LoggerConfig &loggerConfig);

    static std::string asCCString(Byte reg);

private:

    Mc6809LoggerConfig config;
    std::ofstream logOfs;
    mutable bool isLoggingActive{};
};

#endif // CPULOGGER_INCLUDED

