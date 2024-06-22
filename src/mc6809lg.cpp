/*
    mc6809lg.cpp


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


#include "mc6809lg.h"
#include "mc6809st.h"
#include "mc6809.h"
#include <fmt/format.h>
#include <cassert>


bool Mc6809Logger::doLogging(Word pc) const
{
    if (logOfs.is_open())
    {
        if (!config.startAddr.has_value() || pc == config.startAddr.value())
        {
            isLoggingActive = true;
        }

        if (config.stopAddr.has_value() && pc == config.stopAddr.value())
        {
            isLoggingActive = false;
        }

        return (isLoggingActive &&
            (config.minAddr.has_value() && pc >= config.minAddr.value()) &&
            (config.maxAddr.has_value() && pc <= config.maxAddr.value()));
    }

    return false;
}

void Mc6809Logger::logCurrentState(const CpuStatus &cpuState)
{
    if (!logOfs.is_open() || !isLoggingActive)
    {
        return;
    }

    const auto *state = dynamic_cast<const Mc6809CpuStatus *>(&cpuState);

    assert(state != nullptr);
    if (config.logCycleCount)
    {
        logOfs << fmt::format("{:20} ", state->total_cycles);
    }

    if (config.logRegisters == LogRegister::NONE)
    {
        logOfs << fmt::format("{:04X} {}\n", state->pc, state->mnemonic);
        logOfs.flush();
        return;
    }

    logOfs << fmt::format("{:04X} {: <23}", state->pc, state->mnemonic);

    LogRegister registerBit = LogRegister::CC;
    while (registerBit != LogRegister::NONE)
    {
      registerBit <<= 1;
    }

    if ((config.logRegisters & LogRegister::CC) != LogRegister::NONE)
    {
        logOfs << fmt::format(" CC={}", asCCString(state->cc));
    }
    if ((config.logRegisters & LogRegister::A) != LogRegister::NONE)
    {
        logOfs << fmt::format(" A={:02X}", static_cast<Word>(state->a));
    }
    if ((config.logRegisters & LogRegister::B) != LogRegister::NONE)
    {
        logOfs << fmt::format(" B={:02X}", static_cast<Word>(state->b));
    }
    if ((config.logRegisters & LogRegister::DP) != LogRegister::NONE)
    {
        logOfs << fmt::format(" DP={:02X}", static_cast<Word>(state->dp));
    }
    if ((config.logRegisters & LogRegister::X) != LogRegister::NONE)
    {
        logOfs << fmt::format(" X={:04X}", state->x);
    }
    if ((config.logRegisters & LogRegister::Y) != LogRegister::NONE)
    {
        logOfs << fmt::format(" Y={:04X}", state->y);
    }
    if ((config.logRegisters & LogRegister::U) != LogRegister::NONE)
    {
        logOfs << fmt::format(" U={:04X}", state->u);
    }
    if ((config.logRegisters & LogRegister::S) != LogRegister::NONE)
    {
        logOfs << fmt::format(" S={:04X}", state->s);
    }

    logOfs << "\n";
    logOfs.flush();
}

bool Mc6809Logger::setLoggerConfig(const Mc6809LoggerConfig &loggerConfig)
{
    if (logOfs.is_open())
    {
        logOfs.close();
    }

    config = loggerConfig;

    if (!config.logFileName.empty())
    {
        logOfs.open(config.logFileName, std::ios::out | std::ios::trunc);
        if (!logOfs.is_open())
        {
            // Error when trying to open log file.
            config.logFileName.clear();
        }

        return logOfs.is_open();
    }

    return false;
}

std::string Mc6809Logger::asCCString(Byte reg)
{
    constexpr static std::array<Byte, 8> cc_bitmask = {
        CC_BIT_E, CC_BIT_F, CC_BIT_H, CC_BIT_I,
        CC_BIT_N, CC_BIT_Z, CC_BIT_V, CC_BIT_C,
    };
    const static std::string cc_bitnames = "EFHINZVC";
    std::string result = "--------";

    for (int i = 0; i < 8; ++i)
    {
        if (reg & cc_bitmask[i])
        {
            result[i] = cc_bitnames[i];
        }
    }

    return result;
}

