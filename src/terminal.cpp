/*
    terminal.cpp


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
#include "terminal.h"
#include "mc6809.h"
#include "schedule.h"

// pointer to this instance for signal handling
TerminalIO *TerminalIO::instance = nullptr;                                               
#ifdef HAVE_TERMIOS_H
    struct termios TerminalIO::save_termios;
    bool   TerminalIO::used_serial_io = false;
#endif

TerminalIO::TerminalIO(Mc6809 &x_cpu, Scheduler &x_scheduler) :
                        cpu(x_cpu), scheduler(x_scheduler)
{
    instance = this;
    reset_serial();
}

TerminalIO::~TerminalIO()
{
}

void TerminalIO::init(Word reset_key)
{
    init_terminal_io(reset_key);
}

void TerminalIO::reset_serial()
{
    std::lock_guard<std::mutex> guard(serial_mutex);
    key_buffer_serial.clear();
}

void TerminalIO::reset_terminal_io()
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

void TerminalIO::init_terminal_io(Word reset_key)
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
#if defined(VQUIT) && defined(SIGTERM)
            buf.c_cc[VQUIT] = disable;
            signal(SIGTERM, s_exec_signal);
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
        atexit(reset_terminal_io);
    }

#endif // #ifdef HAVE_TERMIOS_H
}

void TerminalIO::put_char_serial(Byte key)
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
bool TerminalIO::has_key_serial()
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
Byte TerminalIO::read_char_serial()
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
Byte TerminalIO::peek_char_serial()
{
    Byte result = 0x00;

    std::lock_guard<std::mutex> guard(serial_mutex);
    if (!key_buffer_serial.empty())
    {
        result = key_buffer_serial.front();
    }

    return result;
}


void TerminalIO::write_char_serial(Byte value)
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

bool TerminalIO::is_terminal_supported()
{
#ifdef HAVE_TERMIOS_H
    return 1;
#else
    return 0;
#endif
}

void TerminalIO::s_exec_signal(int sig_no)
{
    if (TerminalIO::instance != nullptr)
    {
        TerminalIO::instance->exec_signal(sig_no);
    }
}

void TerminalIO::exec_signal(int sig_no)
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
#if defined(SIGTERM)
        case SIGTERM:
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

