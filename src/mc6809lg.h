/*
    mc6809lg.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2025  W. Schwotzer

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
#include "mc6809st.h"
#include <fstream>
#include <string>
#include <array>
#include <deque>


struct CpuStatus;
struct Mc6809CpuStatus;

class Mc6809Logger
{
public:
    Mc6809Logger() = default;
    Mc6809Logger(const Mc6809Logger &src) = delete;
    Mc6809Logger &operator=(const Mc6809Logger &src) = delete;
    Mc6809Logger(Mc6809Logger &&src) = delete;
    Mc6809Logger &operator=(Mc6809Logger &&src) = delete;
    virtual ~Mc6809Logger();

    bool doLogging(Word pc) const;
    void logCpuState(const CpuStatus &state);
    bool setLoggerConfig(const Mc6809LoggerConfig &loggerConfig);

    static std::string asCCString(Byte reg);

private:

    static constexpr std::array<const char *, 8> registerNames
    {
        "CC", "A", "B", "DP", "X", "Y", "U", "S"
    };

    void checkForActivatingLoopMode(const Mc6809CpuStatus &state);
    void finishLoopMode();
    void logLoopContent();
    void logCpuStatePrivate(const Mc6809CpuStatus &state);
    void logCpuStateToText(const Mc6809CpuStatus &state);
    void logCpuStateToCsv(const Mc6809CpuStatus &state);
    void logQueuedStatesUpTo(
            std::deque<Mc6809CpuStatus>::reverse_iterator &iter);
    void doLoopOptimization(const Mc6809CpuStatus &state);
    size_t getLogRegisterCount() const;
    static Byte swapBits(Byte reg);

    Mc6809LoggerConfig config;
    std::ofstream logOfs;
    mutable bool isLoggingActive{};
    bool doPrintCsvHeader{};
    std::deque<Mc6809CpuStatus> cpuStates;
    std::deque<Mc6809CpuStatus>::reverse_iterator cpuStatesIter;
    bool isLoopModeActive{};
    uint64_t loopRepeatCount{};
};

#endif // CPULOGGER_INCLUDED

