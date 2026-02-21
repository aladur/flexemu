/*
    command.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2026  W. Schwotzer

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
#include "misc1.h"
#include "command.h"
#include "asciictl.h"
#include "cpustate.h"
#include "bobshelp.h"
#include "filecnts.h"
#include "absgui.h"
#include "filecntb.h"
#include "e2floppy.h"
#include "inout.h"
#include "schedule.h"
#include "filfschk.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

Command::Command(
        Inout &p_inout,
        Scheduler &p_scheduler,
        E2floppy &p_fdc,
        const sOptions &p_options)
    : inout(p_inout)
    , scheduler(p_scheduler)
    , fdc(p_fdc)
    , options(p_options)
{
}

void Command::resetIo()
{
    command_index = 0;
    answer_index = 0;
    answer.clear();
}

Byte Command::readIo(Word /*offset*/)
{

    if (!answer.empty())
    {
        const Byte character = answer[answer_index++];

        if (character == '\0')
        {
            answer.clear();
            answer_index = 0;
        }
        else if (character == '\n')
        {
            return CR;
        }
        else
        {
            return character;
        }
    }

    return 0x00;
}

std::string Command::next_token(command_t::iterator &iter, int &count)
{
    std::string token;

    while (iter != std::end(command) && *iter == ' ')
    {
        iter++;
    }

    if (iter != std::end(command) && *iter != '\0')
    {
        ++count;
    }

    while (*iter != '\0' && *iter != ' ')
    {
        token.append(1, *(iter++));
    }

    return token;
}

void Command::writeIo(Word /*offset*/, Byte val)
{
    answer.clear();

    if (command_index < MAX_COMMAND - 1)
    {
        command[command_index++] = static_cast<char>(val);
    }
    else
    {
        command[command_index] = static_cast<char>(val);
    }

    if (val == '\0')
    {
        // MSVC does not support auto *commandIter.
        // NOLINTNEXTLINE(readability-qualified-auto)
        auto commandIter = std::begin(command);
        std::stringstream answer_stream;

        command_index = 0;
        answer_index = 0;
        auto number = static_cast<Word>(INVALID_DRIVE);
        auto count = 0;
        const auto arg1 = flx::rtrim(flx::tolower(
                    next_token(commandIter, count)));
        const auto arg2 = next_token(commandIter, count);
        const auto arg3 = next_token(commandIter, count);
        const auto arg4 = next_token(commandIter, count);
        next_token(commandIter, count);

        switch (count)
        {
            case 1:
                if (arg1.compare("exit") == 0)
                {
                    scheduler.request_new_state(CpuState::Exit);
                    return;
                }

                if (arg1.compare("irq") == 0)
                {
                    Notify(NotifyId::SetIrq);
                    return;
                }

                if (arg1.compare("firq") == 0)
                {
                    Notify(NotifyId::SetFirq);
                    return;
                }

                if (arg1.compare("nmi") == 0)
                {
                    Notify(NotifyId::SetNmi);
                    return;
                }

                if (arg1.compare("terminal") == 0)
                {
                    if (!inout.output_to_terminal())
                    {
                        answer_stream << "EMU error: Unable to switch to "
                                         "terminal mode.\n"
#ifdef UNIX
                                         "flexemu has to be started from "
                                         "within a terminal to support this.";
#endif
#ifdef _WIN32
                                         "On Windows terminal mode is not "
                                         "supported";
#endif

                        answer = answer_stream.str();
                    }
                    return;
                }

                if (arg1.compare("graphic") == 0)
                {
                    if (!inout.output_to_graphic())
                    {
                        answer_stream << "EMU error: Unable to switch to "
                                         "graphic mode.";
                        answer = answer_stream.str();
                    }
                    return;
                }

                if (arg1.compare("freq") == 0)
                {
                    answer_stream << std::fixed << std::setprecision(2)
                                  << scheduler.get_frequency() << " MHz";
                    answer = answer_stream.str();
                    return;
                }

                if (arg1.compare("cycles") == 0)
                {
                    answer_stream << scheduler.get_total_cycles()
                                  << " cycles";
                    answer = answer_stream.str();
                    return;
                }

                if (arg1.compare("info") == 0)
                {
                    for (Word drive_nr = 0; drive_nr <= 3; drive_nr++)
                    {
                        answer_stream << fdc.drive_attributes_string(drive_nr);
                    }

                    answer = answer_stream.str();
                    return;
                }

                if (arg1.compare("sync") == 0)
                {
                    if (!fdc.sync_all_drives())
                    {
                        answer_stream << "EMU error: "
                                         "Unable to sync all drives.";
                        answer = answer_stream.str();
                    }

                    return;
                }

                break;

            case 2:
                if (arg1.compare("freq") == 0)
                {
                    float freq;
                    std::stringstream stream(arg2);

                    if (!(stream >> freq).fail() && freq >= 0.0)
                    {
                        Notify(NotifyId::SetFrequency, &freq);
                    }

                    return;
                }

                {
                    std::stringstream stream(arg2);

                    if ((stream >> number).fail() || number >3)
                    {
                        answer_stream << "EMU parameter error: " << arg2 <<
                                         " is not a valid drive number.";
                        answer = answer_stream.str();
                        return;
                    }
                }

                if (arg1.compare("umount") == 0)
                {
                    if (!fdc.umount_drive(number))
                    {
                        answer_stream << "EMU error: "
                                         "Unable to unmount drive #" <<
                                         number << ".";
                        answer = answer_stream.str();
                    }

                    return;
                }

                if (arg1.compare("info") == 0)
                {
                    answer = fdc.drive_attributes_string(number);
                    return;
                }

                if (arg1.compare("sync") == 0)
                {
                    if (!fdc.sync_drive(number))
                    {
                        answer_stream << "EMU error: "
                                         "Unable to sync drive #" <<
                                         number << ".";
                        answer = answer_stream.str();
                    }

                    return;
                }

                if (arg1.compare("check") == 0)
                {
                    const auto *floppy = fdc.get_drive(number);
                    if (floppy == nullptr)
                    {
                        answer_stream << "drive #" << number << " not ready";
                    }
                    else
                    {
                        FlexDiskCheck check(*floppy, options.fileTimeAccess);
                        answer_stream << "Drive #" << number << " ";
                        if (check.CheckFileSystem())
                        {
                            answer_stream << "is OK.";
                        }
                        else
                        {
                            bool with_nl = false;

                            answer_stream << "has " <<
                                check.GetStatisticsString() << '\n';
                            for (const auto &result : check.GetResult())
                            {
                                if (with_nl)
                                {
                                    answer_stream << '\n';
                                }
                                with_nl = true;
                                answer_stream << "  " << result;
                            }
                        }
                    }
                    answer = answer_stream.str();
                    return;
                }
                break;

            case 3:
                {
                    std::stringstream stream(arg3);

                    if ((stream >> number).fail() || number > 3)
                    {
                        answer_stream << "EMU parameter error: " << arg3 <<
                                         " is not a valid drive number.";
                        answer = answer_stream.str();
                        return;
                    }
                }

                if (arg1.compare("mount") == 0)
                {
                    const auto path = convert_path(arg2);
                    if (!fdc.mount_drive(path, number))
                    {
                        answer_stream << "EMU error: "
                                         "Unable to mount " << path <<
                                         " to drive #" << number << ".";
                        answer = answer_stream.str();
                    }

                    return;
                }

                if (arg1.compare("rmount") == 0)
                {
                    if (!fdc.mount_drive(fs::u8path(arg2), number, MOUNT_RAM))
                    {
                        answer_stream << "EMU error: "
                                         "Unable to mount " << arg2 <<
                                         " to drive #" << number << ".";
                        answer = answer_stream.str();
                    }

                    return;
                }

                break;

            case 4:
                if (arg1.compare("format") == 0)
                {
                    int trk;
                    int sec;
                    DiskType disk_type{};
                    const auto extension =
                        flx::tolower(fs::u8path(arg2).extension().u8string());

                    if (extension.empty())
                    {
                        disk_type = DiskType::Directory;
                    }
                    else if ((extension.compare(".dsk") == 0) ||
                        (extension.compare(".wta") == 0))
                    {
                        disk_type = DiskType::DSK;
                    }
                    else if (extension.compare(".flx") == 0)
                    {
                        disk_type = DiskType::FLX;
                    }
                    else
                    {
                        answer_stream << "EMU parameter error: file extension "
                                         "of '" << arg2 << "' is unsupported.";
                        answer = answer_stream.str();
                        return;
                    }

                    std::stringstream tstream{arg3};
                    if ((tstream >> trk).fail() || trk < 2 || trk > 255)
                    {
                        answer_stream << "EMU parameter error: " << arg3 <<
                                         " is not a valid track count.";
                        answer = answer_stream.str();
                        return;
                    }

                    std::stringstream sstream{arg4};
                    if ((sstream >> sec).fail() || sec < 6 || sec > 255)
                    {
                        answer_stream << "EMU parameter error: " << arg4 <<
                                         " is not a valid sector count.";
                        answer = answer_stream.str();
                        return;
                    }

                    if (!fdc.format_disk(
                        static_cast<SWord>(trk),
                        static_cast<SWord>(sec),
                        arg2, disk_type))
                    {
                        answer_stream << "EMU error: Unable to format " <<
                                         arg2 << ".";
                        answer = answer_stream.str();
                    }

                    return;
                }

                break;
        }
        answer_stream << "Unknown command: " << command.data() << ".";
        answer = answer_stream.str();
    }
}

fs::path Command::convert_path(const std::string& path)
{
#ifdef _WIN32
    auto sPath = path;
    for (auto it = sPath.begin(); it != sPath.end(); ++it)
    {
        if (*it == '|')
        {
            *it = ':';
        }
        if (*it == '/')
        {
            *it = '\\';
        }
    }
    return fs::u8path(sPath);
#else
    return fs::u8path(path);
#endif
}
