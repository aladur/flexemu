/*
    pia2v5.cpp


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

#include "typedefs.h"
#include "pia2v5.h"
#include "mc6809.h"
#include "mdcrtape.h"
#include "flexerr.h"
#include <sstream>
#include <iostream>
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"


#define GET_DELTA_TIME static_cast<float>((cpu.get_cycles() - cycles_cdbg) * \
                                          ORIGINAL_PERIOD)

Pia2V5::Pia2V5(Mc6809 &p_cpu) : cpu(p_cpu)

{
    write_buffer.reserve(256);
    read_buffer.reserve(256);
}

Pia2V5::~Pia2V5()
{
    cdbg.close();
}

void Pia2V5::resetIo()
{
    Mc6821::resetIo();
}

void Pia2V5::writeOutputA(Byte value)
{
    ora = value & ddra;
    static DWord write_count = 0;

    if (debug && !cycles_cdbg)
    {
        cdbg.precision(2);
        cdbg << "time=" << std::fixed << 0.0 << "us\n";
        cycles_cdbg = cpu.get_cycles();
    }

    write_count = (ora == 0xE8 || ora == 0xF8) ? write_count + 1 : 0;

    if (debug && write_count <= 6)
    {
        cdbg << "delta_time=" << GET_DELTA_TIME <<
                "us PC=" << std::uppercase << std::hex << cpu.get_pc() <<
                " MDCR command=";
        if (write_count < 6)
        {
            cdbg << std::hex << static_cast<Word>(ora) << '\n';
        }
        else if (write_count == 6)
        {
            cdbg << "...\n";
        }
        cycles_cdbg = cpu.get_cycles();
    }

    if (drive_idx < 0)
    {
        return;
    }

    switch (ora)
    {
        case 0x48: // Tape Forward/Rewind
            direction = TapeDirection::Forward;

            if (last_ora == 0x08)
            {
                // Tape Rewind
                if (debug)
                {
                    cdbg << "Rewind Tape\n";
                }
                direction = TapeDirection::Rewind;
                cycles_BET = cpu.get_cycles();
                cycles_RDC = cpu.get_cycles();
                delay_RDC = static_cast<QWord>(166.F / ORIGINAL_PERIOD);
                // Prepare for write
                write_buffer.clear();
                write_bit_mask = 0x80;
                write_byte = 0;
            }
            last_ora = ora;
            break;

        case 0x68: // Write end-of-data gap
            if (debug && write_bit_mask != 0x80)
            {
                cdbg << "Warning: write buffer not byte-aligned (0x" <<
                        std::hex << static_cast<Word>(write_bit_mask) << ")\n";
            }
            if (!write_buffer.empty())
            {
                drive[drive_idx]->WriteRecord(write_buffer);
                if (debug)
                {
                    cdbg << "delta_time=" << GET_DELTA_TIME <<
                            "us Write Record size=" <<
                            std::dec << write_buffer.size() << '\n';
                    cycles_cdbg = cpu.get_cycles();
                }
                log_buffer(write_buffer);
                write_buffer.clear();
            }
            write_count = 0;
            last_ora = ora;
            break;

        case 0xe8: // Write data: Start high-bit
            if (last_ora == 0xf8)
            {
                // high-bit detected
                write_byte |= write_bit_mask;
                write_bit_mask >>= 1U;
                last_ora = 0;
                break;
            }
            last_ora = ora;
            break;

        case 0xf8: // Write data: Start low-bit
            if (last_ora == 0xe8)
            {
                // low-bit detected
                write_bit_mask >>= 1U;
                last_ora = 0;
                break;
            }
            last_ora = ora;
            break;

        case 0x08: // pre-byte for Rewind
            last_ora = ora;
            break;

        case 0x40: // ???
            read_mode = ReadMode::Off;
            last_ora = ora;
            direction = TapeDirection::NONE;
            break;

        case 0xc8: // Start read data
            if (direction == TapeDirection::Forward &&
                cpu.get_cycles() - cycles_BET > delay_BET &&
                drive[drive_idx]->GetRecordType() != RecordType::NONE &&
                read_mode != ReadMode::Init)
            {
                if (debug)
                {
                    cdbg << "delta_time=" << GET_DELTA_TIME <<
                            "us Enter read mode\n";
                    cycles_cdbg = cpu.get_cycles();
                }

                // Prepare for read
                SetReadModeToInit();
            }
            break;

        default:
            last_ora = ora;
            break;
    }

    if (!write_bit_mask)
    {
        write_buffer.push_back(write_byte);
        write_bit_mask = 0x80;
        write_byte = 0;
    }
}

Byte Pia2V5::readInputA()
{
    last_ora = 0;
    Byte result = ora;
    result |= 0x01U; // Default: set data bit

    if (drive_idx < 0)
    {
        return result;
    }

    result |= 0x02U; // Cassette in position (CIP)
    if (!drive[drive_idx]->IsWriteProtected())
    {
        result |= 0x04U; // No write protection (WPRT)
    }

    if (cpu.get_cycles() - cycles_RDC > delay_RDC &&
        (read_mode == ReadMode::Init && BTST<Byte>(cra, 7U)))
    {
        result &= ~0x01U;
        read_mode = ReadMode::Read;
        cycles_RDC = cpu.get_cycles();

        if (drive[drive_idx]->GetRecordIndex() > 0)
        {
            delay_RDC = static_cast<QWord>(600000.F / ORIGINAL_PERIOD);
        }
        else
        {
            delay_RDC = static_cast<QWord>(166.F / ORIGINAL_PERIOD);
        }
    }
    else if (cpu.get_cycles() - cycles_RDC > delay_RDC &&
             (read_mode == ReadMode::Read && BTST<Byte>(cra, 7U)))
    {
        cycles_RDC = cpu.get_cycles();
        delay_RDC = static_cast<QWord>(166.F / ORIGINAL_PERIOD);

        // Only read a bit if Read Clock (RDC) has been signaled.
        if (read_buffer.empty())
        {
            bool success = drive[drive_idx]->ReadRecord(read_buffer);

            if (debug)
            {
                cdbg << "delta_time=" << GET_DELTA_TIME <<
                        "us Read record " <<
                     (success ? "size=" : "failed") << std::dec <<
                     (success ? std::to_string(read_buffer.size()) : "")
                     << '\n';
                cycles_cdbg = cpu.get_cycles();
            }

            if (success)
            {
                read_index = 0;
                read_bit_mask = 0x80;
                log_buffer(read_buffer);
            }
            else
            {
                read_mode = ReadMode::Off;
            }
        }

        if (read_mode == ReadMode::Read)
        {
            if (read_buffer[read_index] & read_bit_mask)
            {
                // If bit is set clear Read Data (/RDA) (low-active)
                result &= ~0x01U;
            }

            read_bit_mask >>= 1U;
            if (read_bit_mask == 0)
            {
                read_bit_mask = 0x80;
                read_index++;

                if (read_index > read_buffer.size())
                {
                    SetReadModeToInit();
                }
            }
        }

        if (debug && read_mode == ReadMode::Off)
        {
            cdbg << "cycles_delta=" << std::dec <<
                    cpu.get_cycles() - cycles_cdbg <<
                    " Finish read mode\n";
            cycles_cdbg = cpu.get_cycles();
        }
    }

    return result;
}

void Pia2V5::writeOutputB(Byte value)
{
    orb = value & ddrb;

    drive_idx = -1;

    if ((orb & 0x06U) == 0x04U) // low-active Drive #0 selected
    {
        drive_idx = 0;
    }
    else if ((orb & 0x06U) == 0x02U) // low-active Drive #1 selected
    {
        drive_idx = 1;
    }

    if (drive_idx >= 0 && (!drive[drive_idx] || !drive[drive_idx]->IsOpen()))
    {
        // No MDCR tape object assigned or tape file could not be opened.
        drive_idx = -1;
    }

    if (debug)
    {
        cdbg << "delta_time=" << GET_DELTA_TIME <<
                "us PC=" << std::uppercase << std::hex << cpu.get_pc();

        switch(drive_idx)
        {
            case 0:
            case 1:
                cdbg << " Select drive #" << drive_idx << '\n';
                break;

            default:
                cdbg << " Deselect all drives\n";
                break;
        }

        cycles_cdbg = cpu.get_cycles();
    }
}

Byte Pia2V5::readInputB()
{
    return orb;
}

void Pia2V5::requestInputA()
{
    if (cpu.get_cycles() - cycles_BET > delay_BET)
    {
        if (direction == TapeDirection::Rewind && drive_idx >= 0)
        {
            // Rewind record by record.
            // If there is no previous record signal Begin-of-Tape.
            bool begin_of_tape = !drive[drive_idx]->GotoPreviousRecord();

            if (begin_of_tape)
            {
                activeTransition(ControlLine::CA2); // Set /BET
                if (debug)
                {
                    cdbg << "delta_time=" << GET_DELTA_TIME <<
                            "us PC=" << std::uppercase << std::hex <<
                            cpu.get_pc() << " Detected Begin-End-of-Tape\n";
                    cycles_cdbg = cpu.get_cycles();
                    cdbg << std::dec;
                }
            }
            else
            {
                cycles_BET = cpu.get_cycles();
            }

        }
    }

    if (read_mode != ReadMode::Off || direction == TapeDirection::Rewind)
    {
        if (cpu.get_cycles() - cycles_RDC > delay_RDC)
        {
            activeTransition(ControlLine::CA1); // Set Read clock (RDC)
        }
    }
}

void Pia2V5::disk_directory(const char *p_disk_dir)
{
        disk_dir = p_disk_dir;
}

void Pia2V5::SetReadModeToInit()
{
    read_mode = ReadMode::Init;
    read_buffer.clear();
    read_index = 0;
    read_bit_mask = 0x80;
    cycles_RDC = cpu.get_cycles();

    if ((drive[drive_idx]->GetRecordType() == RecordType::Header) &&
        (drive[drive_idx]->GetRecordIndex() > 0))
    {
        // When reading the header of the next file
        // there is an extra long delay
        delay_RDC = static_cast<QWord>(900000.F / ORIGINAL_PERIOD);
    }
    else
    {
        delay_RDC = static_cast<QWord>(120000.F / ORIGINAL_PERIOD);
    }
}

void Pia2V5::mount_all_drives(const std::array<std::string, 2> &paths)
{
    Word drive_nr = 0U;

    for (const auto &path : paths)
    {
        mount_drive(path.c_str(), drive_nr);
        ++drive_nr;
    }

    drive_idx = -1;
}

bool Pia2V5::mount_drive(const char *path, Word drive_nr)
{
    if (drive_nr > 1 || path == nullptr || path[0] == '\0')
    {
        return false;
    }

    // check if already mounted
    if (drive[drive_nr].get() != nullptr)
    {
        return false;
    }

    int i;
    std::string containerPath = path;

    for (i = 0; i < 2; ++i)
    {
        try
        {
            drive[drive_nr] = MiniDcrTape::Open(containerPath);
            if (debug)
            {
                cdbg << "drive_nr=" << drive_nr <<
                        " path=" << containerPath << '\n';
            }
            return true;
        }
        catch (FlexException &exception)
        {
            if (debug)
            {
                cdbg << "drive_nr=" << drive_nr <<
                        " exception.what=" << exception.what() << '\n';
            }
        }

        containerPath = disk_dir;

        if (!containerPath.empty() &&
        containerPath[containerPath.length()-1] != PATHSEPARATOR)
        {
            containerPath += PATHSEPARATORSTRING;
        }

        containerPath += path;
    }

    return false;
}

void Pia2V5::set_debug(const std::string &debugLevel,
                       std::string logFilePath)
{
    if (logFilePath.empty())
    {
        logFilePath = flx::getTempPath() + "/flexemu_mdcr.log";
    }

    std::stringstream streamDebugLevel(debugLevel);
    streamDebugLevel >> debug;

    if (debug)
    {
        if (cdbg.is_open())
        {
            cdbg.close();
        }
        cdbg.open(logFilePath, std::ios::out);
    }
}

void Pia2V5::log_buffer(const std::vector<Byte> &buffer)
{
    if (debug > 1)
    {
        size_t index;

        for(index = 0; index < buffer.size(); ++index)
        {
            cdbg << fmt::format(" {:02X}", static_cast<Word>(buffer[index]));
            if (((index % 16 == 15) && index < (buffer.size() - 1)) ||
               (index == (buffer.size() - 1)))
            {
                cdbg << '\n';
            }
        }
    }
}

