/*
    apprun.cpp


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
#include <new>
#include <sstream>
#include <thread>
#ifdef _MSC_VER
    #include <new.h>
#endif
#ifndef _WIN32
    #include <sched.h>
#endif
#include <sys/types.h>
#ifndef _MSC_VER
    #include <unistd.h>
#endif

#include "e2.h"
#include "apprun.h"
#include "win32gui.h"
#include "xtgui.h"
#include "foptman.h"
#include "flexerr.h"
#include "fileread.h"
#include "btimer.h"


ApplicationRunner::ApplicationRunner(
                    struct sGuiOptions &x_guiOptions,
                    struct sOptions &x_options) :
    guiOptions(x_guiOptions),
    options(x_options),
    memory(options.isHiMem),
    cpu(memory),
    rtc(cpu),
    inout(fdc, rtc),
    scheduler(cpu, inout),
    terminalIO(cpu, scheduler),
    mmu(memory),
    acia1(terminalIO, cpu),
    pia1(cpu, scheduler, keyboardIO),
    pia2(cpu, keyboardIO, joystickIO),
    drisel(fdc),
    command(inout, cpu, scheduler, fdc),
    vico1(memory),
    vico2(memory)
{
    ioDevices.insert({ mmu.getName(), mmu });
    ioDevices.insert({ acia1.getName(), acia1 });
    ioDevices.insert({ pia1.getName(), pia1 });
    ioDevices.insert({ pia2.getName(), pia2 });
    ioDevices.insert({ fdc.getName(), fdc });
    ioDevices.insert({ drisel.getName(), drisel });
    ioDevices.insert({ command.getName(), command });
    ioDevices.insert({ vico1.getName(), vico1 });
    ioDevices.insert({ vico2.getName(), vico2 });
    ioDevices.insert({ rtc.getName(), rtc });
}

ApplicationRunner::~ApplicationRunner()
{
    BTimer::Destroy();
}

int ApplicationRunner::run()
{
    static const auto ioDeviceMappings = std::vector<sIoDeviceMapping>{
        { "mmu", MMU_BASE, -1 },
        { "acia1", ACIA1_BASE, -1 },
        { "pia1", PIA1_BASE, -1 },
        { "pia2", PIA2_BASE, -1 },
        { "fdc", FDC_BASE, -1 },
        // drisel: Same register is mirrored 4 times in address space.
        { "drisel", DRISEL_BASE, 4 },
        { "command", COMM_BASE, -1 },
        { "vico1", VICO1_BASE, -1 },
        { "vico2", VICO2_BASE, -1 },
        // MC146818: Only part of the device is mapped into memory space.
        { "rtc", RTC_BASE, RTC_HIGH - RTC_BASE + 1 },
    };

    cpu.set_disassembler(&disassembler);
    cpu.set_use_undocumented(options.use_undocumented);

    fdc.disk_directory(options.disk_dir.c_str());
    fdc.mount_all_drives(options.drive);

    terminalIO.init(options.reset_key);

    if (!(options.term_mode && terminalIO.is_terminal_supported()))
    {
        inout.create_gui(
                joystickIO,
                keyboardIO,
                terminalIO,
                pia1,
                memory,
                scheduler,
                cpu,
                vico1,
                vico2,
                guiOptions);
    }


    // Add all memory mapped I/O devices to memory.
    for (auto ioDeviceMapping : ioDeviceMappings)
    {
        if (ioDevices.find(ioDeviceMapping.name) != ioDevices.end())
        {
            auto &ioDevice = ioDevices.at(ioDeviceMapping.name);

            memory.add_io_device(ioDevice,
                    ioDeviceMapping.baseAddress, ioDeviceMapping.byteSize);
        }
    }

    // Load monitor program into ROM.
    int error;
    if ((error = load_hexfile(options.hex_file.c_str(), memory)) < 0)
    {
        std::string hexFilePath;

        hexFilePath = options.disk_dir + PATHSEPARATORSTRING +
                      options.hex_file;

        if ((error = load_hexfile(hexFilePath.c_str(), memory)) < 0)
        {
            std::stringstream pmsg;

            pmsg << "File \"" << hexFilePath
                 << "\" not found or has unknown file format (" << error <<")"
                 << std::endl;
#ifdef _WIN32
            MessageBox(nullptr, pmsg.str().c_str(), PROGRAMNAME " error",
            MB_OK | MB_ICONERROR);
#endif
#ifdef UNIX
            fprintf(stderr, "%s", pmsg.str().c_str());
#endif
            return 1;
        }
    }

    memory.reset_io();
    cpu.reset();

    // start CPU thread
    std::thread cpu_thread(&Scheduler::run, &scheduler);

    inout.main_loop();

    cpu_thread.join();  // wait for termination of CPU thread

    return 0;
}

