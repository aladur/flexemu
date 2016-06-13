/*
    inout.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2004  W. Schwotzer

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


#include <misc1.h>

#include "inout.h"
#include "e2floppy.h"
#include "mc6821.h"
#include "mc146818.h"
#include "absgui.h"
#include "bmutex.h"
#include "cacttrns.h"
#include "schedule.h"

#ifdef HAVE_XTK
    #include "xtgui.h"
#endif

#ifdef WIN32
    #include "win32gui.h"
#endif


// pointer to this instance for signal handling
Inout *Inout::instance = NULL;

#ifdef HAVE_TERMIOS_H
    struct termios Inout::save_termios;
    bool   Inout::used_serial_io = false;
#endif

// signal handlers

void Inout::s_exec_signal(int sig_no)
{
    if (Inout::instance != NULL)
    {
        Inout::instance->exec_signal(sig_no);
    }
}


Inout::Inout(Mc6809 *x_cpu, struct sGuiOptions *x_options) :
    cpu(x_cpu), options(x_options), gui(NULL),
    fdc(NULL), memory(NULL), rtc(NULL), pia1(NULL), pia2(NULL),
    video(NULL), schedy(NULL), pmutex(NULL), jmutex(NULL)
{
    reset();
    instance = this;
    pmutex = new BMutex;
    jmutex = new BMutex;
    memset(key_buffer, 0, sizeof(key_buffer));
    memset(key_buffer_ser, 0, sizeof(key_buffer_ser));
}

Inout::~Inout(void)
{
    delete jmutex;
    delete pmutex;
}

void Inout::reset()
{
    reset_parallel();
    reset_serial();
    reset_joystick();
}

void Inout::reset_parallel()
{
    in              = 0;
    out             = 0;
    is_parallel_ch_in_queue = false;
}

void Inout::reset_serial()
{
    in_ser      = 0;
    out_ser     = 0;
}

void Inout::reset_joystick()
{
    deltaX           = 0;
    deltaY           = 0;
    buttonMask       = 0;
    newValues        = 0;
}

void Inout::get_drive_status(tDiskStatus status[4])
{
    if (fdc != NULL)
    {
        fdc->get_drive_status(status);
    }
}

BString Inout::get_drive_info(int floppyIndex)
{
    if (fdc != NULL)
    {
        return fdc->drive_info(floppyIndex);
    }

    return "";
}

bool Inout::get_joystick(int *pDeltaX, int *pDeltaY, unsigned int *pButtonMask)
{
    bool result;

    jmutex->lock();
    result = newValues;

    if (pDeltaX     != NULL)
    {
        *pDeltaX     = deltaX;
    }

    if (pDeltaY     != NULL)
    {
        *pDeltaY     = deltaY;
    }

    if (pButtonMask != NULL)
    {
        *pButtonMask = buttonMask;
    }

    newValues  = false;
    jmutex->unlock();
    return result;
}

void Inout::put_joystick(int x_deltaX, int x_deltaY)
{
    jmutex->lock();
    deltaX     = x_deltaX;
    deltaY     = x_deltaY;
    newValues  = true;
    jmutex->unlock();
}

void Inout::put_joystick(unsigned int x_buttonMask)
{
    buttonMask = x_buttonMask;
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

            if (schedy != NULL)
            {
                schedy->set_new_state(S_EXIT);
            }
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

            if (schedy != NULL)
            {
                schedy->set_new_state(S_EXIT);
            }

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

void Inout::set_gui(AbstractGui *x_gui)
{
    gui = x_gui;
}

void Inout::set_fdc(E2floppy *x_device)
{
    fdc = x_device;
}

void Inout::set_rtc(Mc146818 *x_device)
{
    rtc = x_device;
}

void Inout::set_pia1(Mc6821 *x_device)
{
    pia1 = x_device;
}

void Inout::set_pia2(Mc6821 *x_device)
{
    pia2 = x_device;
}

void Inout::set_video(E2video *x_video)
{
    video = x_video;
}

void Inout::set_memory(Memory *x_memory)
{
    memory = x_memory;
}

void Inout::set_scheduler(Scheduler *x_sched)
{
    schedy = x_sched;
}

AbstractGui *Inout::create_gui(int
#ifndef UNIT_TEST
    type
#endif
                              )
{
#ifndef UNIT_TEST

    if (video != NULL)
    {
        if (gui != NULL && gui->gui_type() != type && (
#ifdef HAVE_XTK
                type == GUI_XTOOLKIT ||
#endif
#ifdef WIN32
                type == GUI_WINDOWS ||
#endif
                type == -9999))   // dummy
        {
            delete gui;
            gui = NULL;
        }

        if (gui == NULL)
        {
            switch (type)
            {
#ifdef HAVE_XTK

                case GUI_XTOOLKIT:
                    gui = new XtGui(cpu, memory, schedy, this, video, options);
                    break;
#endif
#ifdef WIN32

                case GUI_WINDOWS:
                    gui = new Win32Gui(cpu, memory, schedy, this, video,
                                       options);
                    break;
#endif
            } // switch
        } // if
    } // if

    return gui;
#else
    return NULL;
#endif
}

// one second updates are generated by the cpu
// in this method they will be transmitted to all objects
// which need it
void Inout::update_1_second()
{
    if (rtc != NULL)
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
            if (cpu != NULL)
            {
                cpu->set_nmi();
            }

            break;
#if defined(SIGUSR1)

        case SIGUSR1:
            if (cpu != NULL)
            {
                cpu->set_irq();
            }

            break;
#endif
#if defined(SIGUSR2)

        case SIGUSR2:
            if (cpu != NULL)
            {
                cpu->set_firq();
            }

            break;
#endif
#if defined(SIGQUIT)

        case SIGQUIT:
            if (schedy != NULL)
            {
                schedy->set_new_state(S_EXIT);
            }

            break;
#endif
#if defined(SIGTSTP)

        case SIGTSTP:
            if (schedy != NULL)
            {
                schedy->set_new_state(S_RESET_RUN);
            }

            break;
#endif
    }
}

void Inout::put_ch(Byte key)
{
    pmutex->lock();

    if (!key_buffer_full())
    {
        key_buffer[in++] = key;
        is_parallel_ch_in_queue = true;

        if (pia1 != NULL)
        {
            schedy->sync_exec(new CActiveTransition(*pia1, CA1));
        }

        if (in >= KEY_BUFFER_SIZE)
        {
            in = 0;
        }
    }  // if

    pmutex->unlock();
} // put_ch

bool Inout::key_buffer_full(void)
{
    return ((in == out - 1) || (out == 0 && (in == KEY_BUFFER_SIZE - 1)));
}

bool Inout::poll(void)
{
    return is_parallel_ch_in_queue;
}

// input should always be polled before read_ch
Byte Inout::read_ch(void)
{
    Byte result = 0x00;

    pmutex->lock();

    // in == out should never happen, return a dummy value
    if (is_parallel_ch_in_queue) // check for empty buffer
    {
        result = key_buffer[out++];

        if (out >= KEY_BUFFER_SIZE)
        {
            out = 0;
        }

        if (in == out)
        {
            is_parallel_ch_in_queue = false;
        }
        else

            // if there are still characters in the
            // buffer set CA1 flag again
            if (pia1 != NULL)
            {
                schedy->sync_exec(new CActiveTransition(*pia1, CA1));
            }
    }

    pmutex->unlock();

    return result;
}

// read character, but leave it in the queue
Byte Inout::read_queued_ch(void)
{
    Byte result = 0x00;

    pmutex->lock();

    // in == out should never happen, return a dummy value
    if (is_parallel_ch_in_queue) // check for empty buffer
    {
        result = key_buffer[out];
    }

    pmutex->unlock();

    return result;
}

void Inout::put_ch_serial(Byte key)
{
    // convert back space character
#ifdef HAVE_TERMIOS_H
#ifdef VERASE

    if (key == save_termios.c_cc[VERASE] || key == 0x7f)
    {
        key = BACK_SPACE;
    }

#endif
#endif // #ifdef HAVE_TERMIOS_H

    if (!key_buffer_full_serial())
    {
        key_buffer_ser[in_ser++] = key;

        if (in_ser >= KEY_BUFFER_SIZE)
        {
            in_ser = 0;
        }
    }  // if
} // put_ch_serial

Byte Inout::key_buffer_full_serial(void)
{
    return ((in_ser == out_ser - 1) ||
            (out_ser == 0 && (in_ser == KEY_BUFFER_SIZE - 1)));
}

// poll serial port for input
bool Inout::poll_serial(void)
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
            put_ch_serial(buf[0]);
        }
    }

    return (in_ser != out_ser);
#else
    return false;
#endif // #ifdef HAVE_TERMIOS_H
}

// read a serial byte from cpu
// ATTENTION: input should always be polled before read_ch_ser
Byte Inout::read_ch_serial(void)
{
    Byte temp;

    while (in_ser == out_ser)    // check for empty buffer
    {
        return 0x00;    // should never happen, return a dummy value
    }

    temp = key_buffer_ser[out_ser++];

    if (out_ser >= KEY_BUFFER_SIZE)
    {
        out_ser = 0;
    }

    return temp;
}

// read character, but leave it in the queue
Byte Inout::read_queued_ch_serial(void)
{
    while (in_ser == out_ser)    // check for empty buffer
    {
        return 0x00;    // should never happen, return a dummy value
    }

    return key_buffer_ser[out];
}


void Inout::write_ch_serial(Byte val)
{
#ifdef HAVE_TERMIOS_H
    used_serial_io = true;
#ifdef VERASE

    if (val == BACK_SPACE)
    {
        const char *str = "\b \b";

        write(fileno(stdout), str, strlen(str));
        //      putc('\b', stdout);
        //      putc(' ', stdout);
        //      putc('\b', stdout);
    }
    else
#endif
        write(fileno(stdout), &val, 1);

    //      putc(val, stdout);
    //  if (val < ' ' || val > 0x7e)
    //      fflush(stdout);
#endif // #ifdef HAVE_TERMIOS_H
}

void Inout::set_bell(Word /*x_percent*/)
{
#ifdef WIN32
    Beep(400, 100);
#endif
#ifdef UNIX
    static char bell = BELL;

    write(fileno(stdout), &bell, 1);
#endif
}

bool Inout::is_terminal_supported(void)
{
#ifdef HAVE_TERMIOS_H
    return 1;
#else
    return 0;
#endif
}

Word Inout::output_to_terminal(void)
{
#ifdef HAVE_TERMIOS_H

    if (gui != NULL)
    {
        gui->output_to_terminal();
    }

    return 1;
#else
    return 0;
#endif // #ifdef HAVE_TERMIOS_H
}

Word Inout::output_to_graphic(void)
{
    if (gui != NULL)
    {
        gui->output_to_graphic();
        return 1;
    }

    return 0;
}

