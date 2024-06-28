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
#include "scpulog.h"
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

void Mc6809Logger::logCurrentState(const CpuStatus &state)
{
    if (!logOfs.is_open() || !isLoggingActive)
    {
        return;
    }

    const auto *p_state = dynamic_cast<const Mc6809CpuStatus *>(&state);
    assert(p_state != nullptr);

    switch (config.format)
    {
        case Mc6809LoggerConfig::Format::Text:
            logCurrentStateToText(p_state);
            break;

        case Mc6809LoggerConfig::Format::Csv:
            logCurrentStateToCsv(p_state);
            break;
    }
}

void Mc6809Logger::logCurrentStateToText(const Mc6809CpuStatus *state)
{
    assert(state != nullptr);

    if (config.logCycleCount)
    {
        logOfs << fmt::format("{:20} ", state->total_cycles);
    }

    if (config.logRegisters == LogRegister::NONE)
    {
        if (state->operands[0] == '\0')
        {
            logOfs << fmt::format("{:04X} {}\n", state->pc, state->mnemonic);
        }
        else
        {
            logOfs << fmt::format("{:04X} {:<5} {}\n", state->pc,
                    state->mnemonic, state->operands);
        }
        logOfs.flush();
        return;
    }

    auto mnemonic_str = fmt::format("{:<5} {}", state->mnemonic,
            state->operands);
    logOfs << fmt::format("{:04X} {:<24}", state->pc, mnemonic_str);

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

void Mc6809Logger::logCurrentStateToCsv(const Mc6809CpuStatus *state)
{
    assert(state != nullptr);
    const char sep = config.csvSeparator;

    if (doPrintCsvHeader)
    {
        if (config.logCycleCount)
        {
            logOfs << "cycles" << sep;
        }
        logOfs << "PC" << sep << "mnemonic" << sep << "operands";

        LogRegister registerBit = LogRegister::CC;

        for(const char *registerName : registerNames)
        {
            if ((registerBit & config.logRegisters) != LogRegister::NONE)
            {
                logOfs << sep << registerName;
            }
            registerBit <<= 1;
        }
        logOfs << "\n";

        doPrintCsvHeader = false;
    }

    if (config.logCycleCount)
    {
        logOfs << fmt::format("{}{}", state->total_cycles, sep);
    }

    if (config.logRegisters == LogRegister::NONE)
    {
        logOfs << fmt::format("{:04X}{}{}{}{}\n", state->pc, sep,
                state->mnemonic, sep, state->operands);
        logOfs.flush();
        return;
    }

    logOfs << fmt::format("{:04X}{}{}{}{}", state->pc, sep, state->mnemonic,
                sep, state->operands);

    if ((config.logRegisters & LogRegister::CC) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{}", sep, asCCString(state->cc));
    }
    if ((config.logRegisters & LogRegister::A) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{:02X}", sep, static_cast<Word>(state->a));
    }
    if ((config.logRegisters & LogRegister::B) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{:02X}", sep, static_cast<Word>(state->b));
    }
    if ((config.logRegisters & LogRegister::DP) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{:02X}", sep, static_cast<Word>(state->dp));
    }
    if ((config.logRegisters & LogRegister::X) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{:04X}", sep, state->x);
    }
    if ((config.logRegisters & LogRegister::Y) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{:04X}", sep, state->y);
    }
    if ((config.logRegisters & LogRegister::U) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{:04X}", sep, state->u);
    }
    if ((config.logRegisters & LogRegister::S) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{:04X}", sep, state->s);
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
    if (config.format == Mc6809LoggerConfig::Format::Csv)
    {
        doPrintCsvHeader = true;
    }

    if (config.isEnabled)
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

    for (size_t i = 0; i < cc_bitmask.size(); ++i)
    {
        if (reg & cc_bitmask[i])
        {
            result[i] = cc_bitnames[i];
        }
    }

    return result;
}

