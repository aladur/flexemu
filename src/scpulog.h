/*
    scpulog.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2024  W. Schwotzer

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
#include "boption.h"
#include <string>

enum class LogRegister : Byte
{
    NONE = 0,
    CC = (1 << 0),
    A = (1 << 1),
    B = (1 << 2),
    DP = (1 << 3),
    X = (1 << 4),
    Y = (1 << 5),
    U = (1 << 6),
    S = (1 << 7),
};

inline LogRegister operator| (LogRegister lhs, LogRegister rhs)
{
    using TYPE = std::underlying_type<LogRegister>::type;

    return static_cast<LogRegister>(static_cast<TYPE>(lhs) |
                                    static_cast<TYPE>(rhs));
}

inline LogRegister operator& (LogRegister lhs, LogRegister rhs)
{
    using TYPE = std::underlying_type<LogRegister>::type;

    return static_cast<LogRegister>(static_cast<TYPE>(lhs) &
                                    static_cast<TYPE>(rhs));
}

inline LogRegister operator<< (LogRegister lhs, int shift_count)
{
    using TYPE = std::underlying_type<LogRegister>::type;

    return static_cast<LogRegister>(static_cast<TYPE>(lhs) << shift_count);
}

inline LogRegister operator|= (LogRegister &lhs, LogRegister rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

inline LogRegister operator&= (LogRegister &lhs, LogRegister rhs)
{
    lhs = lhs & rhs;
    return lhs;
}

inline LogRegister operator<<= (LogRegister &lhs, int shift_count)
{
    lhs = lhs << shift_count;
    return lhs;
}

struct s_cpu_logfile
{
public:
    s_cpu_logfile()
    {
        logFileName.reserve(PATH_MAX);
    }

    s_cpu_logfile(const s_cpu_logfile &src) = default;
    s_cpu_logfile &operator=(const s_cpu_logfile &src) = default;

    void reset()
    {
        minAddr = 0x0000;
        maxAddr = 0xFFFF;
        startAddr.reset();
        stopAddr.reset();
        logCycleCount = false;
        logRegisters = LogRegister::NONE;
        logFileName.clear();
    }

    s_cpu_logfile(s_cpu_logfile &&src) noexcept
        : minAddr(src.minAddr)
        , maxAddr(src.maxAddr)
        , startAddr(src.startAddr)
        , stopAddr(src.stopAddr)
        , logCycleCount(src.logCycleCount)
        , logRegisters(src.logRegisters)
        , logFileName(std::move(src.logFileName))
    {
    }

    s_cpu_logfile &operator=(s_cpu_logfile &&src) noexcept
    {
        minAddr = src.minAddr;
        maxAddr = src.maxAddr;
        startAddr = src.startAddr;
        stopAddr = src.stopAddr;
        logCycleCount = src.logCycleCount;
        logRegisters = src.logRegisters;
        logFileName = std::move(src.logFileName);
        return *this;
    }

    BOptionalWord minAddr{0x0000};
    BOptionalWord maxAddr{0xFFFF};
    BOptionalWord startAddr;
    BOptionalWord stopAddr;
    bool logCycleCount{false};
    LogRegister logRegisters{LogRegister::NONE};
    std::string logFileName;
};

#endif

