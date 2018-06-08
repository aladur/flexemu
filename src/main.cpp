/*
    main.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

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
#include "mc6809.h"
#include "inout.h"
#include "e2video.h"
#include "pia2.h"
#include "da6809.h"
#include "acia1.h"
#include "mc146818.h"
#include "e2floppy.h"
#include "command.h"
#include "mmu.h"
#include "pia1.h"
#include "win32gui.h"
#include "foptman.h"
#include "absgui.h"
#include "schedule.h"
#include "flexerr.h"
#include "fileread.h"


// define an exception handler when new fails

#ifdef _MSC_VER
// with MSC it's possible to retry memory allocation
int std_new_handler(size_t n)
{
    int result;

    result = MessageBox(NULL, gMemoryAllocationErrorString,
                        PROGRAMNAME " error", MB_RETRYCANCEL | MB_ICONWARNING);

    if (result == IDRETRY)
    {
        return 1;    // retry once more
    }

    throw std::bad_alloc();
    return 0;
}
#else
void std_new_handler()
{
    throw std::bad_alloc();
}
#endif

bool startup(
    Mc6809             **cpu,
    Scheduler      **schedy,
    Inout              **io,
    Da6809             **disassembler,
    struct sGuiOptions *pGuiOptions,
    struct sOptions    *pOptions)
{
    E2video         *video;
    IoDevice        *device;
    Command         *comm;
    Memory      *memory;
    int error;

    memory            = new Memory(pOptions->isHiMem);
    *cpu              = new Mc6809(memory);
    *disassembler     = new Da6809();
    *io               = new Inout(*cpu, pGuiOptions);
    *schedy           = new Scheduler(pOptions);
    (*schedy)->set_cpu(*cpu);
    (*schedy)->set_inout(*io);
    (*io)->set_scheduler(*schedy);
    (*io)->set_memory(memory);
    (*cpu)->set_disassembler(*disassembler);
    (*cpu)->set_use_undocumented(pOptions->use_undocumented);

    // instanciate all memory mapped I/O devices

    device            = new Mmu(*io, memory);
    memory->add_io_device(device, MMU_BASE, MMU_MASK, 0, 0);
    device            = new Acia1(*io, *cpu);
    memory->add_io_device(device, ACIA1_BASE, ACIA1_MASK, 0, 0);
    device            = new Pia1(*io, *cpu);
    memory->add_io_device(device, PIA1_BASE, PIA1_MASK, 0, 0);
    (*io)->set_pia1((Mc6821 *)device);
    device            = new Pia2(*io, *cpu);
    memory->add_io_device(device, PIA2_BASE, PIA2_MASK, 0, 0);
    E2floppy *fdc     = new E2floppy();
    fdc->disk_directory(pOptions->disk_dir.c_str());
    fdc->mount_all_drives(pOptions->drive);
    memory->add_io_device(fdc, FDCA_BASE, FDCA_MASK,
                           FDCB_BASE, FDCB_MASK);
    (*io)->set_fdc(fdc);
    comm              = new Command(*io, *cpu, *schedy);
    memory->add_io_device(comm, COMM_BASE, COMM_MASK, 0, 0);
    comm->set_fdc(fdc);
    video             = new E2video(memory);
    memory->add_io_device(video, VICO_BASE, VICO_MASK, 0, 0);
    (*io)->set_video(video);

    (*io)->init(pOptions->reset_key);

    if (!(pOptions->term_mode && (*io)->is_terminal_supported()))
    {
        (*io)->create_gui(pGuiOptions->guiType);
    }

    // instanciate real time clock right before initialize alarm
    device   = new Mc146818(*cpu);
    memory->add_io_device(device, RTC_LOW, RTC_HIGH - RTC_LOW + 1, 0, 0);
    (*io)->set_rtc((Mc146818 *)device);

    if ((error = load_hexfile(pOptions->hex_file.c_str(), *memory)) < 0)
    {
        //pOptions->hex_file.index(PATHSEPARATOR) < 0) {
        std::string hexFilePath;

        hexFilePath = pOptions->disk_dir + PATHSEPARATORSTRING +
                      pOptions->hex_file;

        if ((error = load_hexfile(hexFilePath.c_str(), *memory)) < 0)
        {
            std::stringstream pmsg;

            pmsg << "File \"" << hexFilePath
                 << "\" not found or has unknown file format (" << error <<")"
                 << std::endl;
#ifdef _WIN32
            MessageBox(NULL, pmsg.str().c_str(), PROGRAMNAME " error",
            MB_OK | MB_ICONERROR);
#endif
#ifdef UNIX
            fprintf(stderr, "%s", pmsg.str().c_str());
#endif
            return false;
        }
    }

    memory->reset_io();
    (*cpu)->reset();
    return true;
} // startup

#ifdef UNIX
int main(int argc, char *argv[])
{
    struct           sOptions    *pOptions;
    struct           sGuiOptions *pGuiOptions;
    Mc6809          *cpu;
    Scheduler   *schedy;
    Inout           *io;
    Da6809          *disassembler;
    FlexOptionManager optionMan;
    int         exit_code = 0;

    std::set_new_handler(std_new_handler);

    pOptions    = new sOptions;
    pGuiOptions = new sGuiOptions;

    try
    {
        optionMan.InitOptions(pGuiOptions, pOptions,
                              argc, argv);
        optionMan.GetOptions(pGuiOptions, pOptions);
        optionMan.GetEnvironmentOptions(pGuiOptions, pOptions);
        optionMan.GetCommandlineOptions(pGuiOptions, pOptions,
                                        argc, argv);
        // write options but only if options file not already exists
        optionMan.WriteOptions(pGuiOptions, pOptions, true);

        if (startup(&cpu, &schedy, &io, &disassembler,
                    pGuiOptions, pOptions))
        {
            // start CPU thread
            if (!schedy->Start())
            {
                fprintf(stderr, "Unable to start CPU thread\n");
                exit_code = 1;
            }
            else
            {
                io->main_loop();

                schedy->Join();  // wait for termination of CPU thread
            }
        }
        else
        {
            // there was an error during startup
            exit_code = 1;
        }
    }
    catch (std::exception &ex)
    {
        fprintf(stderr, PROGRAMNAME ": An error has occured: %s\n", ex.what());
        exit_code = 1;
    }
    catch (FlexException &ex)
    {
        fprintf(stderr, PROGRAMNAME ": An error has occured: %s\n",
            ex.what().c_str());
        exit_code = 1;
    }

    delete io;
    delete schedy;
    delete cpu;
    delete pOptions;
    delete pGuiOptions;

    exit(exit_code);
    return 0; // satisfy compiler
}
#endif

#ifdef _WIN32
void scanCmdLine(LPSTR lpCmdLine, int *argc, char **argv)
{
    *argc = 1;
    *(argv + 0) = "flexemu";

    while (*lpCmdLine)
    {
        *(argv + *argc) = lpCmdLine;

        while (*lpCmdLine && *lpCmdLine != ' ' && *lpCmdLine != '\t')
        {
            lpCmdLine++;
        }

        if (*lpCmdLine)
        {
            *(lpCmdLine++) = '\0';
        }

        while (*lpCmdLine && (*lpCmdLine == ' ' || *lpCmdLine == '\t'))
        {
            lpCmdLine++;
        }

        (*argc)++;
    }
} // scanCmdLine

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    struct           sOptions    *pOptions = NULL;
    struct           sGuiOptions *pGuiOptions = NULL;
    Mc6809          *cpu = NULL;
    Scheduler       *schedy = NULL;
    Inout           *io = NULL;
    Da6809          *disassembler = NULL;
    int             argc;
    char            *argv[50];
    FlexOptionManager optionMan;
    int             exit_code = EXIT_SUCCESS;

    pOptions    = new sOptions;
    pGuiOptions = new sGuiOptions;

    pGuiOptions->hInstance = hInstance;
    pGuiOptions->nCmdShow  = nCmdShow;

#ifdef _MSC_VER
    _PNH oldHandler = set_new_handler(std_new_handler);
#endif

    try
    {
        scanCmdLine(lpCmdLine, &argc, (char **)argv);
        optionMan.InitOptions(pGuiOptions, pOptions, argc, argv);
        optionMan.GetOptions(pGuiOptions, pOptions);
        optionMan.GetEnvironmentOptions(pGuiOptions, pOptions);
        optionMan.GetCommandlineOptions(pGuiOptions, pOptions,
                                        argc, argv);
        // write options but only if options file not already exists
        optionMan.WriteOptions(pGuiOptions, pOptions, true);

        if (startup(&cpu, &schedy, &io, &disassembler,
                    pGuiOptions, pOptions))
        {
            // start CPU thread
            if (!schedy->Start())
            {
                MessageBox(NULL, "Unable to start CPU thread",
                           PROGRAMNAME " error", MB_OK | MB_ICONERROR);
                exit_code = EXIT_FAILURE;
            }
            else
            {
                io->main_loop();

                // wait until CPU thread has terminated
                schedy->Join();
            }
        }
        else
        {
            exit_code = EXIT_FAILURE;
        }
    }
    catch (std::bad_alloc UNUSED(&e))
    {
        MessageBox(NULL, gMemoryAllocationErrorString,
                   PROGRAMNAME " error", MB_OK | MB_ICONERROR);
        exit_code = EXIT_FAILURE;
    }

    delete io;
    delete schedy;
    delete cpu;
    delete pOptions;
    delete pGuiOptions;

    return exit_code; // satisfy compiler
} // WinMain

#endif // #ifdef _WIN32

