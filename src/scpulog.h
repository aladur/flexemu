/*
    scpulog.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2025  W. Schwotzer

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

#include "typedefs.h"
#include "warnoff.h"
#include <optional>
#include "warnon.h"
#include <filesystem>

namespace fs = std::filesystem;


using OptionalWord = std::optional<Word>;
enum class LogRegister : uint8_t
{
    NONE = 0U,
    CC = (1U << 0U),
    A = (1U << 1U),
    B = (1U << 2U),
    DP = (1U << 3U),
    X = (1U << 4U),
    Y = (1U << 5U),
    U = (1U << 6U),
    S = (1U << 7U),
};

inline LogRegister operator| (LogRegister lhs, LogRegister rhs)
{
    using TYPE = std::underlying_type_t<LogRegister>;

    return static_cast<LogRegister>(static_cast<TYPE>(lhs) |
                                    static_cast<TYPE>(rhs));
}

inline LogRegister operator& (LogRegister lhs, LogRegister rhs)
{
    using TYPE = std::underlying_type_t<LogRegister>;

    return static_cast<LogRegister>(static_cast<TYPE>(lhs) &
                                    static_cast<TYPE>(rhs));
}

inline LogRegister operator<< (LogRegister lhs, unsigned shift_count)
{
    using TYPE = std::underlying_type_t<LogRegister>;

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

struct Mc6809LoggerConfig
{
    enum class Format : uint8_t
    {
        Csv,
        Text,
    };

public:
    Mc6809LoggerConfig() = default;
    Mc6809LoggerConfig(const Mc6809LoggerConfig &src) = default;
    Mc6809LoggerConfig &operator=(const Mc6809LoggerConfig &src) = default;

    void reset()
    {
        minAddr = static_cast<Word>(0x0000U);
        maxAddr = static_cast<Word>(0xFFFFU);
        startAddr.reset();
        stopAddr.reset();
        logCycleCount = false;
        logRegisters = LogRegister::NONE;
        logFilePath.clear();
        format = Format::Text;
        csvSeparator = ';';
        isEnabled = false;
        isLoopOptimization = false;
    }

    Mc6809LoggerConfig(Mc6809LoggerConfig &&src) noexcept
        : minAddr(src.minAddr)
        , maxAddr(src.maxAddr)
        , startAddr(src.startAddr)
        , stopAddr(src.stopAddr)
        , logCycleCount(src.logCycleCount)
        , logRegisters(src.logRegisters)
        , logFilePath(std::move(src.logFilePath))
        , format(src.format)
        , csvSeparator(src.csvSeparator)
        , isEnabled(src.isEnabled)
        , isLoopOptimization(src.isLoopOptimization)
    {
    }

    Mc6809LoggerConfig &operator=(Mc6809LoggerConfig &&src) noexcept
    {
        minAddr = src.minAddr;
        maxAddr = src.maxAddr;
        startAddr = src.startAddr;
        stopAddr = src.stopAddr;
        logCycleCount = src.logCycleCount;
        logRegisters = src.logRegisters;
        logFilePath = std::move(src.logFilePath);
        format = src.format;
        csvSeparator = src.csvSeparator;
        isEnabled = src.isEnabled;
        isLoopOptimization = src.isLoopOptimization;
        return *this;
    }

    OptionalWord minAddr{static_cast<Word>(0x0000U)};
    OptionalWord maxAddr{static_cast<Word>(0xFFFFU)};
    OptionalWord startAddr;
    OptionalWord stopAddr;
    bool logCycleCount{false};
    LogRegister logRegisters{LogRegister::NONE};
    fs::path logFilePath;
    Format format{Format::Text};
    char csvSeparator{';'};
    bool isEnabled{false};
    bool isLoopOptimization{true};
};

#endif

