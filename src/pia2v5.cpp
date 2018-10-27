/*
    pia2v5.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018  W. Schwotzer

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

#include "misc1.h"

#ifdef HAVE_X11
    #include <X11/X.h>
#endif
#include "pia2v5.h"
#include "mc6809.h"
#include "mdcrtape.h"
#include <sstream>
#include <iostream>
#include <iomanip>


Pia2V5::Pia2V5(Mc6809 &x_cpu) : cpu(x_cpu), last_ora(0), delay_BET(0),
    read_bit_mask(0x80), read_index(0), delay_read(0),
    write_bit_mask(0x80), write_byte(0), drive_idx(-1), read_mode(false),
    direction(TapeDirection::NONE), debug(0)
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

    if (ora == 0xE8 || ora == 0xF8)
    {
        write_count++;
    }
    else
    {
        write_count = 0;
    }

    if (debug)
    { 
        if (write_count < 6)
        {
            cdbg << "PC=" << std::uppercase << std::hex << cpu.get_pc() <<
                     " MDCR command=" <<
                     std::hex << (unsigned int)ora << std::endl;
        }
        else if (write_count == 6)
        {
            cdbg << "PC=" << std::uppercase << std::hex << cpu.get_pc() <<
                     " MDCR command=..." << std::endl;
        }
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
                    cdbg << "Rewind Tape" << std::endl;
                }
                direction = TapeDirection::Rewind;
                delay_BET = default_delay_BET;
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
                        std::hex << (unsigned int)write_bit_mask << ")" <<
                        std::endl;
            }
            if (!write_buffer.empty())
            {
                drive[drive_idx]->WriteRecord(write_buffer);
                if (debug)
                {
                    cdbg << "Write Record size=" <<
                            std::dec << write_buffer.size() <<
                            std::endl;
                }
                log_buffer(write_buffer);
                write_buffer.clear();
            }
            last_ora = ora;
            break;

        case 0xe8: // Write data: Start high-bit
            if (last_ora == 0xf8)
            {
                // high-bit detected
                write_byte |= write_bit_mask;
                write_bit_mask >>= 1;
                last_ora = 0;
                break;
            }
            last_ora = ora;
            break;

        case 0xf8: // Write data: Start low-bit
            if (last_ora == 0xe8)
            {
                // low-bit detected
                write_bit_mask >>= 1;
                last_ora = 0;
                break;
            }
            last_ora = ora;
            break;

        case 0x08: // pre-byte for Rewind
            last_ora = ora;
            break;

        case 0x40: // ???
            last_ora = ora;
            direction = TapeDirection::NONE;
            break;

        case 0xc8: // Start read data
            if (direction == TapeDirection::Rewind)
            {
                if (!drive[drive_idx]->GotoPreviousRecord())
                {
                    // Begin-of-Tape: Signal it after a delay.
                    delay_BET = default_delay_BET;
                }
            }

            if (!delay_BET && drive[drive_idx]->HasRecord())
            {
                if (debug)
                {
                    cdbg << "Enter read mode" << std::endl;
                }
                read_mode = true;
                read_buffer.clear();
                read_index = 0;
                read_bit_mask = 0x80;
                delay_read = 10;
                delay_BET = 0;
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

    if (drive_idx >= 0)
    {
        result |= 0x02; // Cassette in position (CIP)
        if (!drive[drive_idx]->IsWriteProtected())
        {
            result |= 0x04;  // No write protection (WPRT)
        }

        if (read_mode)
        {
            if (!delay_read || (delay_read && !--delay_read))
            {
                if (read_buffer.empty())
                {
                    bool success = drive[drive_idx]->ReadRecord(read_buffer);

                    if (debug)
                    {
                        cdbg << "Read record " <<
                             (success ? "size=" : "failed") << std::dec <<
                             (success ? std::to_string(read_buffer.size()) : "")
                             << std::endl;
                    }

                    if (success)
                    {
                        log_buffer(read_buffer);
                    }
                    else
                    {
                        read_mode = false;
                    }
                }

                if (read_mode)
                {
                    if (!(read_buffer[read_index] & read_bit_mask))
                    {
                        // If bit is clear set Read Data (/RDA) low-active
                        result |= 0x01;
                    }

                    read_bit_mask >>= 1;
                    if (read_bit_mask == 0)
                    {
                        read_bit_mask = 0x80;
                        read_index++;
                        if (read_index > read_buffer.size())
                        {
                            read_mode = false;
                        }
                    }
                }
            }

            if (debug && !read_mode)
            {
                cdbg << "Finish read mode" << std::endl;
            }
        }
    }

    return result;
}

void Pia2V5::writeOutputB(Byte value)
{
    orb = value & ddrb;

    drive_idx = -1;

    if ((orb & 0x06) == 0x04) // low-active Drive #0 selected
    {
        drive_idx = 0;
    }
    else if ((orb & 0x06) == 0x02) // low-active Drive #1 selected
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
        switch(drive_idx)
        {
            case 0:
            case 1:
                cdbg << "PC=" << std::uppercase << std::hex <<
                        cpu.get_pc() << " Select drive #" << drive_idx <<
                        std::endl;
                break;

            default:
                cdbg << "PC=" << std::uppercase << std::hex <<
                    cpu.get_pc() << " Deselect all drives" <<
                    std::endl;
                break;
        }
    }
}

Byte Pia2V5::readInputB()
{
    return orb;
}

void Pia2V5::requestInputA()
{
    if (delay_BET && !--delay_BET)
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
                    cdbg << "PC=" << std::uppercase << std::hex <<
                            cpu.get_pc() << " Detected Begin-End-of-Tape" <<
                            std::endl;
                }
            }
            else
            {
                delay_BET = default_delay_BET;
            }

        }
    }

    if (read_mode || direction == TapeDirection::Rewind)
    {
        activeTransition(ControlLine::CA1); // Set Read clock (RDC)
    }
}

void Pia2V5::disk_directory(const char *x_disk_dir)                           
{
        disk_dir = x_disk_dir;
}

void Pia2V5::mount_all_drives(std::array<std::string, 2> drives)
{
    Word drive_nr;

    for (drive_nr = 0; drive_nr < drives.size(); drive_nr++)
    {
        mount_drive(drives[drive_nr].c_str(), drive_nr);
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
        cdbg << "path=" << containerPath << std::endl;
        drive[drive_nr] = MiniDcrTape::Create(containerPath.c_str());

        if (drive[drive_nr].get() != nullptr)
        {
            return true;
        }

        std::string containerPath = disk_dir;

        if (containerPath.length() > 0 &&
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
        // On POSIX compliant file systems /tmp has to be available
        std::string tmpPath = "/tmp";
 
#ifdef _WIN32
        char cTempPath[MAX_PATH];
        if (!GetTempPath(MAX_PATH, cTempPath))
        {
            throw FlexException(GetLastError(),
                   std::string("In function GetTempPath"));
        }
        tmpPath = cTempPath;
#endif
        logFilePath = tmpPath + "/flexemu_mdcr.log";                      
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
            cdbg << " " << std::hex << std::uppercase << std::setw(2) <<
                    std::setfill('0') << static_cast<Word>(buffer[index]);
            if (((index % 16 == 15) && index < (buffer.size() - 1)) ||
               (index == (buffer.size() - 1)))
            {
                cdbg << std::endl;
            }
        }
    }
}

