/*
    apprun.cpp


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

#include "misc1.h"
#include <new>
#include <sstream>
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
#include "foptman.h"
#include "flexerr.h"
#include "fileread.h"
#include "fcnffile.h"
#include "iodevdbg.h"
#include "soptions.h"
#include "qtgui.h"
#include "cvtwchar.h"


ApplicationRunner::ApplicationRunner(struct sOptions &p_options) :
    options(p_options),
    memory(options),
    cpu(memory),
    fdc(options),
    inout(p_options, memory),
    scheduler(cpu, inout),
    terminalIO(scheduler, p_options),
    mmu(memory),
    acia1(terminalIO, inout),
    pia1(scheduler, keyboardIO, p_options),
    pia2(cpu, keyboardIO, joystickIO),
    pia2v5(cpu),
    drisel(fdc),
    command(inout, scheduler, fdc),
    gui(cpu, memory, scheduler, inout, vico1, vico2,
        joystickIO, keyboardIO, terminalIO, pia1, p_options)
{
    if (options.startup_command.size() > MAX_COMMAND)
    {
        std::stringstream message;

        message << "Startup command exceeds " << MAX_COMMAND << " characters";
        throw std::invalid_argument(message.str());
    }

    // neumnt54.hex is obsolete now.
    // neumon54.hex can be used for both terminal and GUI mode.
    // SERPAR flag is switched dynamically during emulation.
    if (getFileName(options.hex_file) == std::string("neumnt54.hex"))
    {
        std::stringstream message;

        message << "Also for terminal mode neumon54.hex can be used now!";
        throw std::invalid_argument(message.str());
    }

    if (options.isEurocom2V5)
    {
        // Eurocom II/V5 is always emulated without RAM extension and RTC.
        options.isRamExtension = false;
        options.useRtc = false;
    }

    if (options.isRamExtension)
    {
        ioDevices.insert({ mmu.getName(), mmu });
    }
    else
    {
        // If no RAM extension is present:
        // Limit the number of columns to 2.
        options.nColors = 2;
        // Switch of High memory option.
        options.isHiMem = false;
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
        gui.SetFloppy(&fdc);
    }
    ioDevices.insert({ command.getName(), command });
    ioDevices.insert({ vico1.getName(), vico1 });
    ioDevices.insert({ vico2.getName(), vico2 });
    if (options.useRtc)
    {
        ioDevices.insert({ rtc.getName(), rtc });
        inout.set_rtc(&rtc);
    }

    scheduler.set_frequency(options.frequency);

    FlexemuConfigFile configFile(getFlexemuSystemConfigFile().c_str());
    auto address = configFile.GetSerparAddress(options.hex_file);
    if (address < 0 || !terminalIO.is_terminal_supported())
    {
        // The specified hex_file does not support switching between
        // serial/parallel input/output or it is unknown how to switch.
        // Or terminal mode is not support in general.
        // In any of these cases terminal mode has to be switched off.
        options.term_mode = false;
    }
    inout.serpar_address(address);

    if (options.isEurocom2V5)
    {
        auto logMdcr = configFile.GetDebugSupportOption("logMdcr");
        auto logFilePath = configFile.GetDebugSupportOption("logMdcrFilePath");
        pia2v5.set_debug(logMdcr, logFilePath);
    }

    pia1.Attach(inout);
    pia1.Attach(cpu);
    acia1.Attach(cpu);
    terminalIO.Attach(cpu);
    command.Attach(cpu);
    vico1.Attach(memory);
    vico2.Attach(memory);
    if (options.useRtc)
    {
        rtc.Attach(cpu);
    }
}

ApplicationRunner::~ApplicationRunner()
{
    cleanup();
}

void ApplicationRunner::AddIoDevicesToMemory()
{
    FlexemuConfigFile configFile(getFlexemuSystemConfigFile().c_str());
    const auto deviceParams = configFile.ReadIoDevices();
    const auto pairOfParams = configFile.GetIoDeviceLogging();
    const auto logFilePath = std::get<0>(pairOfParams);
    const auto deviceNames = std::get<1>(pairOfParams);

    // Reserve space for all devices otherwise references get invalidated.
    debugLogDevices.reserve(deviceParams.size());

    // Add all memory mapped I/O devices to memory.
    for (const auto &deviceParam : deviceParams)
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
    DWord startAddress = 0;

    int error = load_hexfile(options.hex_file, memory, startAddress);
    if (error < 0)
    {
        std::string hexFilePath;

        hexFilePath = options.disk_dir + PATHSEPARATORSTRING +
                      options.hex_file;

        error = load_hexfile(hexFilePath, memory, startAddress);
        if (error < 0)
        {
            std::stringstream pmsg;

            pmsg << "*** Error in \"" << hexFilePath << "\":\n    ";
            print_hexfile_error(pmsg, error);
            pmsg << '\n';
#ifdef _WIN32
            MessageBox(
                nullptr,
                ConvertToUtf16String(pmsg.str()).c_str(),
                ConvertToUtf16String(PROGRAMNAME " error").c_str(),
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

int ApplicationRunner::startup(QApplication &app)
{
    cpu.set_disassembler(&disassembler);
    cpu.set_use_undocumented(options.use_undocumented);

    if (options.isEurocom2V5)
    {
        pia2v5.disk_directory(options.disk_dir.c_str());
        pia2v5.mount_all_drives(options.mdcrDrives);
    }
    else
    {
        fdc.disk_directory(options.disk_dir);
        fdc.mount_all_drives(options.drives);
    }

    terminalIO.init(options.reset_key);
    inout.set_gui(&gui);

    if (!(options.term_mode && terminalIO.is_terminal_supported()))
    {
        gui.show();
    }

    AddIoDevicesToMemory();

    if (!LoadMonitorFileIntoRom())
    {
        return 1;
    }

    memory.reset_io();
    cpu.reset();

    if (options.term_mode && terminalIO.is_terminal_supported())
    {
        terminalIO.set_startup_command(options.startup_command.c_str());
    }
    else
    {
        keyboardIO.set_startup_command(options.startup_command.c_str());
    }

    // start CPU thread
    cpuThread = std::make_unique<std::thread>(&Scheduler::run, &scheduler);

    QObject::connect(&gui, &QtGui::CloseApplication, &app,
                     &QCoreApplication::quit, Qt::QueuedConnection);

    return 0;
}

void ApplicationRunner::cleanup()
{
    if (cpuThread)
    {
        // Make sure that the CPU thread leaves the suspended state
        // otherwise join will end up in an endless state.
        scheduler.request_new_state(CpuState::Exit);
        cpuThread->join(); // wait for termination of CPU thread
        cpuThread.reset();
    }
}

