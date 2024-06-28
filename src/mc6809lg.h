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
#include <array>


class CpuStatus;
class Mc6809CpuStatus;

class Mc6809Logger
{
public:
    Mc6809Logger() = default;
    virtual ~Mc6809Logger() = default;

    bool doLogging(Word pc) const;
    void logCurrentState(const CpuStatus &state);
    bool setLoggerConfig(const Mc6809LoggerConfig &loggerConfig);

    static std::string asCCString(Byte reg);

private:

    static constexpr std::array<const char *, 8> registerNames
    {
        "CC", "A", "B", "DP", "X", "Y", "U", "S"
    };

    void logCurrentStateToText(const Mc6809CpuStatus *state);
    void logCurrentStateToCsv(const Mc6809CpuStatus *state);

    Mc6809LoggerConfig config;
    std::ofstream logOfs;
    mutable bool isLoggingActive{};
    bool doPrintCsvHeader{};
};

#endif // CPULOGGER_INCLUDED
