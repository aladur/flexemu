/*
    apprun.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2019  W. Schwotzer

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
#include "fcnffile.h"
#include "iodevdbg.h"


ApplicationRunner::ApplicationRunner(
                    struct sGuiOptions &x_guiOptions,
                    struct sOptions &x_options) :
    guiOptions(x_guiOptions),
    options(x_options),
    memory(options),
    cpu(memory),
    rtc(cpu),
    inout(),
    scheduler(cpu, inout),
    terminalIO(cpu, scheduler),
    mmu(memory),
    acia1(terminalIO, cpu),
    pia1(cpu, scheduler, keyboardIO, x_options.isEurocom2V5),
    pia2(cpu, keyboardIO, joystickIO),
    pia2v5(cpu),
    drisel(fdc),
    command(inout, cpu, scheduler, fdc),
    vico1(memory),
    vico2(memory)
{
    if (options.isEurocom2V5)
    {
        // Eurocom II/V5 is always emulated without RAM extension and RTC.
        options.isRamExtension = false;
        options.useRtc = false;
    }

    if (!options.isRamExtension)
    {
        // If no RAM extension is present:
        // Limit the number of columns to 2.
        guiOptions.nColors = 2;
        // Switch of High memory option.
        options.isHiMem = false;
    }

    if (options.isRamExtension)
    {
        ioDevices.insert({ mmu.getName(), mmu });
    }
    ioDevices.insert({ acia1.getName(), acia1 });
    ioDevices.insert({ pia1.getName(), pia1 });
    if (options.isEurocom2V5)
    {
        ioDevices.insert({ pia2v5.getName(), pia2v5 });
    }
    else
    {
        ioDevices.insert({ pia2.getName(), pia2 });
        ioDevices.insert({ fdc.getName(), fdc });
        ioDevices.insert({ drisel.getName(), drisel });
        inout.set_fdc(&fdc);
        ioDevices.insert({ command.getName(), command });
    }
    ioDevices.insert({ vico1.getName(), vico1 });
    ioDevices.insert({ vico2.getName(), vico2 });
    if (options.useRtc)
    {
        ioDevices.insert({ rtc.getName(), rtc });
        inout.set_rtc(&rtc);
    }

    if (options.frequency >= 0.0f)
    {
        scheduler.set_frequency(options.frequency);
    }

    FlexemuConfigFile configFile(getFlexemuSystemConfigFile().c_str());
    auto address = configFile.GetSerparAddress(options.hex_file.c_str());
    inout.serpar_address(address);

    if (options.isEurocom2V5)
    {
        auto debugMdcr = configFile.GetDebugOption("debugMdcr");
        auto logFilePath = configFile.GetDebugOption("debugMdcrFilePath");
        pia2v5.set_debug(debugMdcr, logFilePath);
    }
}

ApplicationRunner::~ApplicationRunner()
{
    BTimer::Destroy();
}

void ApplicationRunner::AddIoDevicesToMemory()
{
    FlexemuConfigFile configFile(getFlexemuSystemConfigFile().c_str());
    const auto deviceParams = configFile.ReadIoDevices();
    const auto pairOfParams = configFile.GetDebugIoDevices();
    const auto logFilePath = std::get<0>(pairOfParams);
    const auto deviceNames = std::get<1>(pairOfParams);

    // Reserve space for all devices otherwise references get invalidated.
    debugLogDevices.reserve(deviceParams.size());

    // Add all memory mapped I/O devices to memory.
    for (auto deviceParam : deviceParams)
    {
        std::string name = deviceParam.name;

        if (ioDevices.find(name) != ioDevices.end())
        {
            std::reference_wrapper<IoDevice> deviceRef = ioDevices.at(name);

            if (deviceNames.find(name) != deviceNames.end())
            {
                // Wrap the I/O-device by the I/O-device logger
                debugLogDevices.emplace_back(std::ref(deviceRef), logFilePath);
                auto lastPos = debugLogDevices.size() - 1;
                deviceRef =
                      dynamic_cast<IoDevice &>(debugLogDevices.at(lastPos));
            }

            memory.add_io_device(std::ref(deviceRef),
                deviceParam.baseAddress, deviceParam.byteSize);
        }
    }

}

bool ApplicationRunner::LoadMonitorFileIntoRom()
{
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
            return false;
        }
    }
    return true;
}

int ApplicationRunner::run()
{
    cpu.set_disassembler(&disassembler);
    cpu.set_use_undocumented(options.use_undocumented);

    fdc.disk_directory(options.disk_dir.c_str());
    fdc.mount_all_drives(options.drive);
    pia2v5.disk_directory(options.disk_dir.c_str());
    pia2v5.mount_all_drives(options.mdcrDrives);

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

    AddIoDevicesToMemory();

    if (!LoadMonitorFileIntoRom())
    {
        return 1;
    }

    memory.reset_io();
    cpu.reset();

    // start CPU thread
    std::thread cpu_thread(&Scheduler::run, &scheduler);

    inout.main_loop();

    cpu_thread.join();  // wait for termination of CPU thread

    return 0;
}

