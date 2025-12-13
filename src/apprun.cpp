/*
    apprun.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2025  W. Schwotzer

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
#include "config.h"
#include <sstream>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "apprun.h"
#include "foptman.h"
#include "fileread.h"
#include "fcnffile.h"
#include "soptions.h"
#include "qtgui.h"
#include "scpulog.h"
#include "warnoff.h"
#include <Qt>
#include <QObject>
#include <QCoreApplication>
#include <QMessageBox>
#include "warnon.h"
#include <cstdlib>
#include <stdexcept>


ApplicationRunner::ApplicationRunner(struct sOptions &p_options,
        ITerminalImplPtr &&termImpl) :
    options(p_options),
    memory(options),
    cpu(memory),
    fdc(options),
    inout(p_options, memory),
    scheduler(cpu, inout),
    terminalIO(scheduler, std::move(termImpl)),
    mmu(memory),
    acia1(terminalIO, inout),
    pia1(scheduler, keyboardIO, p_options),
    pia2(cpu, keyboardIO, joystickIO),
    pia2v5(cpu),
    drisel(fdc),
    command(inout, scheduler, fdc, options),
    tstdev(512U),
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
    if (options.hex_file.filename().u8string() == std::string("neumnt54.hex"))
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

    if (!options.cpuLogPath.empty())
    {
        Mc6809LoggerConfig loggerConfig;

        loggerConfig.logFilePath = options.cpuLogPath;
        loggerConfig.isEnabled = true;
        loggerConfig.logCycleCount = true;
        const auto extension =
            flx::tolower(options.cpuLogPath.extension().u8string());
        if (extension == ".csv")
        {
            loggerConfig.format = Mc6809LoggerConfig::Format::Csv;
            loggerConfig.csvSeparator = ';';
        }
        else
        {
            loggerConfig.format = Mc6809LoggerConfig::Format::Text;
        }

        cpu.setLoggerConfig(loggerConfig);
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
    ioDevices.insert({ tstdev.getName(), tstdev });

    scheduler.set_frequency(options.frequency);

    if (options.isEurocom2V5)
    {
        FlexemuConfigFile configFile(flx::getFlexemuConfigFile());

        auto logMdcr = configFile.GetDebugSupportOption("logMdcr");
        auto logFilePath =
            fs::u8path(configFile.GetDebugSupportOption("logMdcrFilePath"));
        pia2v5.set_debug(logMdcr, logFilePath);
    }

    pia1.Attach(inout);
    pia1.Attach(cpu);
    acia1.Attach(cpu);
    terminalIO.Attach(cpu);
    terminalIO.Attach(gui);
    command.Attach(cpu);
    command.Attach(gui);
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
    FlexemuConfigFile configFile(flx::getFlexemuConfigFile());
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

int ApplicationRunner::LoadBootRomFile()
{
    auto hexFilePath = options.hex_file;
    DWord startAddress = 0;

    int error = load_hexfile(hexFilePath, memory, startAddress);
    if (error < 0)
    {
        if (!hexFilePath.is_absolute())
        {
            hexFilePath = options.disk_dir / hexFilePath;

            error = load_hexfile(hexFilePath, memory, startAddress);
        }
    }

    if (error == 0)
    {
        return 0;
    }

    QMessageBox::StandardButtons buttons = QMessageBox::Close;
    auto message =
        QObject::tr("Boot ROM file \"%1\"<br>"
        "can not be read or has wrong format.<br>"
        "<br>&#x2022; <b>Close</b> will close flexemu.")
            .arg(QString::fromStdString(hexFilePath.u8string()));

    if (!FlexemuOptions::AreAllBootOptionsReadOnly(options, true))
    {
        message +=
            QObject::tr("<br>&#x2022; <b>Restore Defaults</b> will restore "
                    "boot ROM file to default value and restart flexemu.");
        buttons |= QMessageBox::RestoreDefaults;
    }

#ifdef _WIN32
    const auto registryPath =
        FlexemuOptions::GetFlexemuRegistryConfigPath();
    message += QObject::tr(
            "<br><br><b>Hint:</b> flexemu settings can be changed by "
            "editing the Windows Registry at"
            "<br>%1").arg(QString::fromStdString(registryPath));
#else
    const auto rcFilePath = FlexemuOptions::GetRcFilePath().u8string();
    message += QObject::tr(
            "<br><br><b>Hint:</b> flexemu settings can be changed by "
            "editing file<br>%1").arg(QString::fromStdString(rcFilePath));
#endif

    const auto answer =
        QMessageBox::critical(nullptr, "flexemu error", message, buttons,
                QMessageBox::Close);

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wswitch"
#endif
    switch (answer)
    {
        case QMessageBox::RestoreDefaults:
            FlexemuOptions::InitBootOptions(options, true);
            FlexemuOptions::WriteOptions(options, false, true);
            return EXIT_RESTART;

        case QMessageBox::Close:
        default:
            return 1;
    }
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
}

int ApplicationRunner::startup(QApplication &app)
{
    int exitCode = 0;

    cpu.set_disassembler(&disassembler);
    cpu.set_use_undocumented(options.use_undocumented);

    if (options.isEurocom2V5)
    {
        pia2v5.disk_directory(options.disk_dir);
        pia2v5.mount_all_drives(options.mdcrDrives);
    }
    else
    {
        fdc.disk_directory(options.disk_dir);
        fdc.mount_all_drives(options.drives);
    }

    if (!terminalIO.init(options.reset_key))
    {
        return 1;
    }

    FlexemuConfigFile configFile(flx::getFlexemuConfigFile());

    auto optional_address = configFile.GetSerparAddress(options.hex_file);
    if (!optional_address.has_value() || !terminalIO.is_terminal_supported())
    {
        // The specified hex_file does not support switching between
        // serial/parallel input/output or it is unknown how to switch.
        // Or terminal mode is not support in general.
        // In any of these cases terminal mode has to be switched off.
        options.term_mode = false;
    }
    inout.serpar_address(optional_address);

    inout.set_gui(&gui);

    if (!(options.term_mode && terminalIO.is_terminal_supported()))
    {
        gui.show();
    }

    AddIoDevicesToMemory();

    exitCode = LoadBootRomFile();
    if (exitCode != 0)
    {
        return exitCode;
    }

    memory.reset_io();
    cpu.reset();

    auto optional_boot_char = configFile.GetBootCharacter(options.hex_file);
    if (options.term_mode && terminalIO.is_terminal_supported())
    {
        terminalIO.set_startup_command(options.startup_command);
    }
    else
    {
        keyboardIO.set_startup_command(options.startup_command);
    }
    keyboardIO.set_boot_char(optional_boot_char);

    // start CPU thread
    cpuThread = std::make_unique<std::thread>(&Scheduler::run, &scheduler);

    QObject::connect(&gui, &QtGui::CloseApplication, &app,
                     &QCoreApplication::quit, Qt::QueuedConnection);

    return exitCode;
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

