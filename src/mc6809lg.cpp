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
#include <algorithm>
#include <cassert>


Mc6809Logger::~Mc6809Logger()
{
    finishLoopMode();
}

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

void Mc6809Logger::checkForActivatingLoopMode(const Mc6809CpuStatus &state)
{
    if (!isLoopModeActive)
    {
        const auto iter = std::find_if(cpuStates.cbegin(), cpuStates.cend(),
                [&](const Mc6809CpuStatus &cpuState){
                    return cpuState.pc == state.pc;
                });
        if (iter != cpuStates.cend())
        {
            cpuStates.erase(iter + 1, cpuStates.cend());
            isLoopModeActive = true;
            loopRepeatCount = 0U;
            cpuStatesIter = cpuStates.rbegin();

            for_each(cpuStates.crbegin(), cpuStates.crend(),
                [&](const Mc6809CpuStatus &/* cpuState */){
                });
        }
    }
}

void Mc6809Logger::finishLoopMode()
{
    if (!isLoopModeActive)
    {
        return;
    }

    logLoopContent();

    isLoopModeActive = false;

    if (loopRepeatCount == 1 && cpuStatesIter == cpuStates.rbegin())
    {
        // If only repeated once and loop has finished at the end
        // do a normal logging.
        auto iter = cpuStates.rend();
        logQueuedStatesUpTo(iter);
    }

    if (cpuStatesIter != cpuStates.rbegin())
    {
        // If loop has been quit in the middle do a normal logging.
        logQueuedStatesUpTo(cpuStatesIter);
    }

    loopRepeatCount = 0U;
    cpuStates.clear();
    cpuStatesIter = cpuStates.rend();
}

void Mc6809Logger::doLoopOptimization(const Mc6809CpuStatus &state)
{
    checkForActivatingLoopMode(state);

    if (isLoopModeActive)
    {
        if (cpuStatesIter->pc == state.pc &&
            cpuStatesIter->insn_size == state.insn_size &&
                std::equal(
                    std::begin(cpuStatesIter->instruction),
                    std::begin(cpuStatesIter->instruction) + state.insn_size,
                    std::begin(state.instruction)))
        {
            // Identical PC values and instruction size + bytes, continue loop.
            // The instruction size + bytes have to be checked to
            // detect self modifying code which also has to finish loop mode.
            *(cpuStatesIter++) = state;
            if (cpuStatesIter == cpuStates.rend())
            {
                cpuStatesIter = cpuStates.rbegin();
                ++loopRepeatCount;
            }
        }
        else
        {
            // Different PC values, finish loop mode.
            finishLoopMode();
        }
    }
    else
    {
        cpuStates.emplace_front(state);
    }
}

void Mc6809Logger::logCpuState(const CpuStatus &state)
{
    if (!isLoggingActive || !logOfs.is_open())
    {
        return;
    }

    const auto *p_state = dynamic_cast<const Mc6809CpuStatus *>(&state);
    assert(p_state != nullptr);

    if (config.isLoopOptimization)
    {
        doLoopOptimization(*p_state);
    }

    if (!isLoopModeActive)
    {
        logCpuStatePrivate(*p_state);
    }
}

void Mc6809Logger::logLoopContent()
{
    if (loopRepeatCount == 0 ||
        (loopRepeatCount == 1 && cpuStatesIter == cpuStates.rbegin()))
    {
        return;
    }

    char sep = ' ';
    std::string do_str;
    std::string repeat_str;
    auto fctLogCycleCount = [&](){
        if (config.logCycleCount)
        {
            switch (config.format)
            {
                case Mc6809LoggerConfig::Format::Text:
                    logOfs << fmt::format("{:21}", "");
                    break;

                case Mc6809LoggerConfig::Format::Csv:
                    logOfs << config.csvSeparator;
                    break;
            }
        }
    };
    auto fctLogRegisters = [&](){
        if (config.format == Mc6809LoggerConfig::Format::Csv)
        {
            for(size_t i = 0; i < getLogRegisterCount(); ++i)
            {
                logOfs << sep;
            }
        }
    };

    switch (config.format)
    {
        case Mc6809LoggerConfig::Format::Text:
            do_str = "     DO";
            repeat_str = fmt::format("     REPEAT{}#{}", sep, loopRepeatCount);
            break;

        case Mc6809LoggerConfig::Format::Csv:
            sep = config.csvSeparator;
            do_str = fmt::format("{0}DO{0}", sep);
            repeat_str = fmt::format("{0}REPEAT{0}#{1}", sep, loopRepeatCount);
            break;
    }

    fctLogCycleCount();
    logOfs << do_str;
    fctLogRegisters();
    logOfs << "\n";

    auto iter = cpuStates.rend();
    logQueuedStatesUpTo(iter);

    fctLogCycleCount();
    logOfs << repeat_str;
    fctLogRegisters();
    logOfs << "\n";
    logOfs.flush();
}

void Mc6809Logger::logQueuedStatesUpTo(
        std::deque<Mc6809CpuStatus>::reverse_iterator &iter)
{
    for_each(cpuStates.rbegin(), iter,
        [&](const Mc6809CpuStatus &cpuState){
            logCpuStatePrivate(cpuState);
        });
}

void Mc6809Logger::logCpuStatePrivate(const Mc6809CpuStatus &state)
{
    switch (config.format)
    {
        case Mc6809LoggerConfig::Format::Text:
            logCpuStateToText(state);
            break;

        case Mc6809LoggerConfig::Format::Csv:
            logCpuStateToCsv(state);
            break;
    }
}

void Mc6809Logger::logCpuStateToText(const Mc6809CpuStatus &state)
{
    if (config.logCycleCount)
    {
        logOfs << (isLoopModeActive ?
            fmt::format("{:21}", "") :
            fmt::format("{:20} ", state.total_cycles));
    }

    if (config.logRegisters == LogRegister::NONE)
    {
        logOfs << fmt::format("{:04X} ", state.pc) << state.mnemonic;
        if (state.operands[0] != '\0')
        {
            logOfs << " " << state.operands;
        }
        logOfs << "\n";
        logOfs.flush();
        return;
    }

    const auto mnemo_oper = (state.operands[0] != '\0') ?
        fmt::format("{:<5} {}", state.mnemonic, state.operands) :
        state.mnemonic;

    if (isLoopModeActive)
    {
        logOfs << fmt::format("{:04X} {}\n", state.pc, mnemo_oper);
        logOfs.flush();
        return;
    }

    logOfs << fmt::format("{:04X} {:<24}", state.pc, mnemo_oper);

    if ((config.logRegisters & LogRegister::CC) != LogRegister::NONE)
    {
        logOfs << fmt::format(" CC={}", asCCString(state.cc));
    }
    if ((config.logRegisters & LogRegister::A) != LogRegister::NONE)
    {
        logOfs << fmt::format(" A={:02X}", static_cast<Word>(state.a));
    }
    if ((config.logRegisters & LogRegister::B) != LogRegister::NONE)
    {
        logOfs << fmt::format(" B={:02X}", static_cast<Word>(state.b));
    }
    if ((config.logRegisters & LogRegister::DP) != LogRegister::NONE)
    {
        logOfs << fmt::format(" DP={:02X}", static_cast<Word>(state.dp));
    }
    if ((config.logRegisters & LogRegister::X) != LogRegister::NONE)
    {
        logOfs << fmt::format(" X={:04X}", state.x);
    }
    if ((config.logRegisters & LogRegister::Y) != LogRegister::NONE)
    {
        logOfs << fmt::format(" Y={:04X}", state.y);
    }
    if ((config.logRegisters & LogRegister::U) != LogRegister::NONE)
    {
        logOfs << fmt::format(" U={:04X}", state.u);
    }
    if ((config.logRegisters & LogRegister::S) != LogRegister::NONE)
    {
        logOfs << fmt::format(" S={:04X}", state.s);
    }

    logOfs << "\n";
    logOfs.flush();
}

void Mc6809Logger::logCpuStateToCsv(const Mc6809CpuStatus &state)
{
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
        if (!isLoopModeActive)
        {
            logOfs << state.total_cycles;
        }
        logOfs << sep;
    }

    if (config.logRegisters == LogRegister::NONE)
    {
        logOfs << fmt::format("{:04X}", state.pc) << sep << state.mnemonic;
        if (state.operands[0] != '\0')
        {
            logOfs << sep << state.operands;
        }
        logOfs << "\n";
        logOfs.flush();
        return;
    }

    logOfs << fmt::format("{:04X}", state.pc) << sep << state.mnemonic <<
                sep << state.operands;

    if (isLoopModeActive)
    {
        for (size_t i = 0; i < getLogRegisterCount(); ++i)
        {
            logOfs << sep;
        }
        logOfs << "\n";
        logOfs.flush();
        return;
    }

    if ((config.logRegisters & LogRegister::CC) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{}", sep, asCCString(state.cc));
    }
    if ((config.logRegisters & LogRegister::A) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{:02X}", sep, static_cast<Word>(state.a));
    }
    if ((config.logRegisters & LogRegister::B) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{:02X}", sep, static_cast<Word>(state.b));
    }
    if ((config.logRegisters & LogRegister::DP) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{:02X}", sep, static_cast<Word>(state.dp));
    }
    if ((config.logRegisters & LogRegister::X) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{:04X}", sep, state.x);
    }
    if ((config.logRegisters & LogRegister::Y) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{:04X}", sep, state.y);
    }
    if ((config.logRegisters & LogRegister::U) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{:04X}", sep, state.u);
    }
    if ((config.logRegisters & LogRegister::S) != LogRegister::NONE)
    {
        logOfs << fmt::format("{}{:04X}", sep, state.s);
    }

    logOfs << "\n";
    logOfs.flush();
}

bool Mc6809Logger::setLoggerConfig(const Mc6809LoggerConfig &loggerConfig)
{
    finishLoopMode();

    if (logOfs.is_open())
    {
        logOfs.close();
    }

    config = loggerConfig;
    isLoggingActive = !config.startAddr.has_value();
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

size_t Mc6809Logger::getLogRegisterCount()
{
    size_t count = 0;

    auto registerBit = LogRegister::CC;
    for(size_t i = 0; i < registerNames.size(); ++i)
    {
        if ((registerBit & config.logRegisters) != LogRegister::NONE)
        {
            ++count;
        }
        registerBit <<= 1;
    }

    return count;
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

