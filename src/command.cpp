/*
    command.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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
#ifdef ultrix
    #include <strings.h>
#endif
#include <ctype.h>
#include <new>

#include "e2.h"
#include "command.h"
#include "typedefs.h"
#include "absgui.h"
#include "flexerr.h"
#include "e2floppy.h"
#include "inout.h"
#include "schedule.h"
#include <sstream>
#include <iomanip>


Command::Command(
        Inout &p_inout,
        Scheduler &p_scheduler,
        E2floppy &p_fdc) :
    inout(p_inout), scheduler(p_scheduler), fdc(p_fdc)
{
    memset(command, 0, sizeof(command));
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

const char *Command::next_token(char **pp, int *pcount)
{
    while (*pp != nullptr && **pp != '\0' && **pp == ' ')
    {
        (*pp)++;
    }

    if (*pp != nullptr && **pp != '\0')
    {
        (*pcount)++;
    }

    return *pp;
}


void Command::skip_token(char **pp)
{
    while (*pp != nullptr && **pp != '\0' && **pp != ' ')
    {
        (*pp)++;
    }

    if (*pp != nullptr && **pp == ' ')
    {
        **pp = '\0';
        (*pp)++;
    }
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
        char *p;
        const char *arg1;
        const char *arg2;
        const char *arg3;
        const char *arg4;
        std::stringstream answer_stream;

        command_index = 0;
        answer_index = 0;
        auto number = static_cast<int>(INVALID_DRIVE);
        auto count = 0;
        p = command;
        arg1 = next_token(&p, &count); // get arg1
        skip_token(&p);
        arg2 = next_token(&p, &count); // get arg2
        skip_token(&p);
        arg3 = next_token(&p, &count); // get arg3
        skip_token(&p);
        arg4 = next_token(&p, &count); // get arg4
        skip_token(&p);
        next_token(&p, &count);

        switch (count)
        {
            case 1:
                if (stricmp(arg1, "exit") == 0)
                {
                    scheduler.request_new_state(CpuState::Exit);
                    return;
                }

                if (stricmp(arg1, "irq")  == 0)
                {
                    Notify(NotifyId::SetIrq);
                    return;
                }

                if (stricmp(arg1, "firq")  == 0)
                {
                    Notify(NotifyId::SetFirq);
                    return;
                }

                if (stricmp(arg1, "nmi")  == 0)
                {
                    Notify(NotifyId::SetNmi);
                    return;
                }

                if (stricmp(arg1, "terminal") == 0)
                {
                    inout.output_to_terminal();
                    return;
                }

                if (stricmp(arg1, "graphic") == 0)
                {
                    if (!inout.output_to_graphic())
                    {
                        answer_stream << "EMU error: Unable to change to "
                                         "graphic mode.";
                        answer = answer_stream.str();
                    }
                    return;
                }

                if (stricmp(arg1, "freq") == 0)
                {
                    answer_stream << std::fixed << std::setprecision(2)
                                  << scheduler.get_frequency() << " MHz";
                    answer = answer_stream.str();
                    return;
                }

                if (stricmp(arg1, "cycles") == 0)
                {
                    answer_stream << scheduler.get_total_cycles()
                                  << " cycles";
                    answer = answer_stream.str();
                    return;
                }

                if (stricmp(arg1, "info") == 0)
                {
                    for (Word drive_nr = 0; drive_nr <= 3; drive_nr++)
                    {
                        answer_stream << fdc.drive_info_string(drive_nr);
                    }

                    answer = answer_stream.str();
                    return;
                }

                if (stricmp(arg1, "sync") == 0)
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
                if (stricmp(arg1, "freq") == 0)
                {
                    float freq;

                    if ((sscanf(arg2, "%f", &freq) == 1) &&
                        freq >= 0.0)
                    {
                        scheduler.set_frequency(freq);
                    }

                    return;
                }

                if ((sscanf(arg2, "%d", &number) != 1) ||
                    number < 0 || number > 3)
                {
                    answer_stream << "EMU parameter error: " << arg2 <<
                                     " is not a valid drive number.";
                    answer = answer_stream.str();
                    return;
                }

                if (stricmp(arg1, "umount") == 0)
                {
                    if (!fdc.umount_drive(static_cast<Word>(number)))
                    {
                        answer_stream << "EMU error: "
                                         "Unable to unmount drive #" <<
                                         number << ".";
                        answer = answer_stream.str();
                    }

                    return;
                }

                if (stricmp(arg1, "info") == 0)
                {
                    answer =
                        fdc.drive_info_string(static_cast<Word>(number));
                    return;
                }

                if (stricmp(arg1, "sync") == 0)
                {
                    if (!fdc.sync_drive(static_cast<Word>(number)))
                    {
                        answer_stream << "EMU error: "
                                         "Unable to sync drive #" <<
                                         number << ".";
                        answer = answer_stream.str();
                    }

                    return;
                }

                break;

            case 3:
                if (stricmp(arg1, "mount") == 0)
                {
                    if ((sscanf(arg3, "%d", &number) != 1) ||
                        number < 0 || number > 3)
                    {
                        answer_stream << "EMU parameter error: " << arg3 <<
                                         " is not a valid drive number.";
                        answer = answer_stream.str();
                        return;
                    }

                    if (!fdc.mount_drive(arg2, static_cast<Byte>(number)))
                    {
                        answer_stream << "EMU error: "
                                         "Unable to mount " << arg2 <<
                                         " to drive #" << number << ".";
                        answer = answer_stream.str();
                    }

                    return;
                }

                if (stricmp(arg1, "rmount") == 0)
                {
                    if ((sscanf(arg3, "%d", &number) != 1) ||
                        number < 0 || number > 3)
                    {
                        answer_stream << "EMU parameter error: " << arg3 <<
                                         " is not a valid drive number.";
                        answer = answer_stream.str();
                        return;
                    }

                    if (!fdc.mount_drive(arg2, static_cast<Byte>(number),
                                         MOUNT_RAM))
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
                if (stricmp(arg1, "format") == 0)
                {
                    int trk;
                    int sec;
                    int type = 0;
                    auto extension = getFileExtension(arg2);

                    if (extension.empty())
                    {
                        type = TYPE_NAFS_DIRECTORY;
                    }
                    else if ((stricmp(extension.c_str(), ".dsk") == 0) ||
                        (stricmp(extension.c_str(), ".wta") == 0))
                    {
                        type = TYPE_DSK_CONTAINER;
                    }
                    else if (stricmp(extension.c_str(), ".flx") == 0)
                    {
                        type = TYPE_FLX_CONTAINER;
                    }
                    else
                    {
                        answer_stream << "EMU parameter error: file extension "
                                         "of '" << arg2 << "' is unsupported.";
                        answer = answer_stream.str();
                        return;
                    }

                    if ((sscanf(arg3, "%d", &trk) != 1) ||
                        trk < 2 || trk > 255)
                    {
                        answer_stream << "EMU parameter error: " << arg3 <<
                                         " is not a valid track count.";
                        answer = answer_stream.str();
                        return;
                    }

                    if ((sscanf(arg4, "%d", &sec) != 1) ||
                        sec < 6 || sec > 255)
                    {
                        answer_stream << "EMU parameter error: " << arg4 <<
                                         " is not a valid sector count.";
                        answer = answer_stream.str();
                        return;
                    }

                    if (!fdc.format_disk(
                        static_cast<SWord>(trk),
                        static_cast<SWord>(sec),
                        arg2, type))
                    {
                        answer_stream << "EMU error: Unable to format " <<
                                         arg2 << ".";
                        answer = answer_stream.str();
                    }

                    return;
                }

                break;
        } // switch
        answer_stream << "Unknown command: " << command << ".";
        answer = answer_stream.str();
    } // if
}

