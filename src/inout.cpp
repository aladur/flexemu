/*
    inout.cpp


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

#include "inout.h"
#include "e2floppy.h"
#include "mc6809.h"
#include "mc6821.h"
#include "mc146818.h"
#include "absgui.h"
#include "schedule.h"
#include "joystick.h"

#ifdef HAVE_XTK
    #include "xtgui.h"
#endif

#ifdef _WIN32
    #include "win32gui.h"
#endif


// pointer to this instance for signal handling
Inout *Inout::instance = nullptr;

#ifdef HAVE_TERMIOS_H
    struct termios Inout::save_termios;
    bool   Inout::used_serial_io = false;
#endif

// signal handlers

void Inout::s_exec_signal(int sig_no)
{
    if (Inout::instance != nullptr)
    {
        Inout::instance->exec_signal(sig_no);
    }
}


Inout::Inout(Mc6809 &x_cpu, Scheduler &x_scheduler) :
    cpu(x_cpu), scheduler(x_scheduler), gui(nullptr), fdc(nullptr), rtc(nullptr)
{
    instance = this;
    reset_serial();
}

Inout::~Inout()
{
    instance = nullptr;
    delete gui;
}

void Inout::reset_serial()
{
    std::lock_guard<std::mutex> guard(serial_mutex);
    key_buffer_serial.clear();
}

void Inout::get_drive_status(DiskStatus status[4])
{
    if (fdc != nullptr)
    {
        fdc->get_drive_status(status);
    }
}

std::string Inout::get_drive_info(int floppyIndex)
{
    if (fdc != nullptr)
    {
        return fdc->drive_info(floppyIndex);
    }

    return "";
}

void Inout::init(Word reset_key)
{
    initTerminalIO(reset_key);
}

void Inout::resetTerminalIO()
{
#ifdef HAVE_TERMIOS_H

    if (isatty(fileno(stdin)))
    {
        tcsetattr(0, TCSAFLUSH, &save_termios);

        if (used_serial_io)
        {
            fprintf(stdout, "\n");
        }
    }

#endif // #ifdef HAVE_TERMIOS_H
}

void Inout::initTerminalIO(Word reset_key)
{
#ifdef HAVE_TERMIOS_H
    struct termios  buf;
    tcflag_t    mask;

    if (isatty(fileno(stdin)))
    {
        if (tcgetattr(fileno(stdin), &save_termios) < 0)
        {
            fprintf(stderr, "unable to initialize terminal\n");

            scheduler.set_new_state(S_EXIT);
        }
        else
        {
            buf = save_termios;

            // c_lflag:
            mask = 0
#ifdef ICANON
                   | ICANON
#endif
#ifdef ECHO
                   | ECHO
#endif
#ifdef IEXTEN
                   | IEXTEN
#endif
                   ;
            buf.c_lflag &= ~mask;
#ifdef ISIG
            buf.c_lflag |= ISIG;
#endif

            // c_iflag:
            mask = 0
#ifdef BRKINT
                   | BRKINT
#endif
#ifdef ISTRIP
                   | ISTRIP
#endif
#ifdef IXON
                   | IXON
#endif
#ifdef ICRNL
                   | ICRNL
#endif
                   ;
            buf.c_iflag &= ~mask;

            // test: c_oflag not needed to be changed
            // c_oflag:
            //          mask = 0
#ifdef OPOST
            //              | OPOST
#endif
            //          ;
            //          buf.c_oflag |= mask;
            buf.c_cc[VMIN]  = 0;
            buf.c_cc[VTIME] = 0;
            long disable = fpathconf(fileno(stdin), _PC_VDISABLE);

            if (disable < 0)
            {
                disable = reset_key;
            }

#if defined(SIGUSR1)
            signal(SIGUSR1, s_exec_signal);
#endif
#if defined(SIGUSR2)
            signal(SIGUSR2, s_exec_signal);
#endif
#if defined(VINTR) && defined(SIGINT)
            buf.c_cc[VINTR] = reset_key;
            signal(SIGINT, s_exec_signal);
#endif
#if defined(VQUIT) && defined(SIGQUIT)
            buf.c_cc[VQUIT] = disable;
            signal(SIGQUIT, s_exec_signal);
#endif
#ifdef VSUSP
            buf.c_cc[VSUSP] = disable;
#endif
#if defined(VSUSP) && defined(SIGQUIT)
            buf.c_cc[VSUSP] = disable;
#ifdef VDSUSP
            buf.c_cc[VDSUSP] = disable;
#endif
            signal(SIGTSTP, s_exec_signal);
#endif
        }

        if (tcsetattr(fileno(stdin), TCSAFLUSH, &buf) < 0)
        {
            // on error try to switch back,
            // otherwise terminal is damaged
            tcsetattr(fileno(stdin), TCSAFLUSH, &save_termios);
            fprintf(stderr, "unable to initialize terminal\n");

            scheduler.set_new_state(S_EXIT);

            return;
        }

        // use atexit here to reset the Terminal IO
        // because the X11 interface under some error
        // condition like missing DISPLAY variable or
        // X11 protocol error aborts with exit()
        atexit(resetTerminalIO);
    }

#endif // #ifdef HAVE_TERMIOS_H
}

void Inout::set_fdc(E2floppy *x_device)
{
    fdc = x_device;
}

void Inout::set_rtc(Mc146818 *x_device)
{
    rtc = x_device;
}

AbstractGui *Inout::create_gui(JoystickIO &joystickIO,
                               KeyboardIO &keyboardIO, Pia1 &pia1,
                               Memory &memory, E2video &video,
                               struct sGuiOptions &options)
{
#ifdef UNIT_TEST
    (void)joystickIO;
    (void)keyboardIO;
    (void)pia1;
    (void)memory;
    (void)video;
    (void)options;
#else
    // Only allow to open Gui once.
    if (gui == nullptr)
    {
        switch (static_cast<GuiType>(options.guiType))
        {
#ifdef HAVE_XTK

            case GuiType::XTOOLKIT:
                gui = new XtGui(cpu, memory, scheduler, *this, video,
                                joystickIO, keyboardIO, pia1, options);
                break;
#endif
#ifdef _WIN32

            case GuiType::WINDOWS:
                gui = new Win32Gui(cpu, memory, scheduler, *this, video,
                                   joystickIO, keyboardIO, pia1, options);
                break;
#endif
            default:
                throw FlexException(FERR_UNSUPPORTED_GUI_TYPE,
                        static_cast<int>(options.guiType));
                break;
        }
    }
#endif // UNIT_TEST
    return gui;
}

// one second updates are generated by the cpu
// in this method they will be transmitted to all objects
// which need it
void Inout::update_1_second()
{
    if (rtc != nullptr)
    {
        rtc->update_1_second();
    }
}

void Inout::exec_signal(int sig_no)
{
    signal(sig_no, s_exec_signal); // set handler again

    switch (sig_no)
    {
        case SIGINT:
            cpu.set_nmi();

            break;
#if defined(SIGUSR1)

        case SIGUSR1:
            cpu.set_irq();

            break;
#endif
#if defined(SIGUSR2)

        case SIGUSR2:
            cpu.set_firq();

            break;
#endif
#if defined(SIGQUIT)

        case SIGQUIT:
            scheduler.set_new_state(S_EXIT);

            break;
#endif
#if defined(SIGTSTP)

        case SIGTSTP:
            scheduler.set_new_state(S_RESET_RUN);

            break;
#endif
    }
}

void Inout::put_char_serial(Byte key)
{
    std::lock_guard<std::mutex> guard(serial_mutex);
    // convert back space character
#ifdef HAVE_TERMIOS_H
#ifdef VERASE

    if (key == save_termios.c_cc[VERASE] || key == 0x7f)
    {
        key = BACK_SPACE;
    }

#endif
#endif // #ifdef HAVE_TERMIOS_H

    key_buffer_serial.push_back(key);
}

// poll serial port for input character.
bool Inout::has_key_serial()
{
#ifdef HAVE_TERMIOS_H
    char    buf[1];
    static Word count = 0;

    if (++count >= 100)
    {
        count = 0;
        fflush(stdout);

        if (read(fileno(stdin), &buf, 1) > 0)
        {
            put_char_serial(buf[0]);
        }
    }

    return true;
#else
    return false;
#endif // #ifdef HAVE_TERMIOS_H
}

// Read a serial character from cpu.
// ATTENTION: Input should always be polled before read_char_serial.
Byte Inout::read_char_serial()
{
    Byte result = 0x00;

    std::lock_guard<std::mutex> guard(serial_mutex);
    if (!key_buffer_serial.empty())
    {
        result = key_buffer_serial.front();
        key_buffer_serial.pop_front();
    }

    return result;
}

// Read character, but leave it in the queue.
// ATTENTION: Input should always be polled before peek_char_serial.
Byte Inout::peek_char_serial()
{
    Byte result = 0x00;

    std::lock_guard<std::mutex> guard(serial_mutex);
    if (!key_buffer_serial.empty())
    {
        result = key_buffer_serial.front();
    }

    return result;
}


void Inout::write_char_serial(Byte value)
{
    size_t count = 0;
#ifdef HAVE_TERMIOS_H
    used_serial_io = true;
#ifdef VERASE

    if (value == BACK_SPACE)
    {
        const char *str = "\b \b";

        count = write(fileno(stdout), str, strlen(str));
        //      putc('\b', stdout);
        //      putc(' ', stdout);
        //      putc('\b', stdout);
    }
    else
#endif
    count = write(fileno(stdout), &value, 1);
    (void)count; // satisfy compiler
#endif // #ifdef HAVE_TERMIOS_H
}

bool Inout::is_terminal_supported()
{
#ifdef HAVE_TERMIOS_H
    return 1;
#else
    return 0;
#endif
}

bool Inout::is_gui_present()
{
    return gui != nullptr;
}

Word Inout::output_to_terminal()
{
#ifdef HAVE_TERMIOS_H

    if (gui != nullptr)
    {
        gui->output_to_terminal();
    }

    return 1;
#else
    return 0;
#endif // #ifdef HAVE_TERMIOS_H
}

Word Inout::output_to_graphic()
{
    if (gui != nullptr)
    {
        gui->output_to_graphic();
        return 1;
    }

    return 0;
}

void Inout::main_loop()
{
    if (is_gui_present())
    {
        gui->main_loop();
    }
}
