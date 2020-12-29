/*
    terminal.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2020  W. Schwotzer

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

TerminalIO::TerminalIO(Scheduler &x_scheduler) :
                       scheduler(x_scheduler), init_delay(500)
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
    init_delay = 500;
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

#ifdef HAVE_TERMIOS_H
void TerminalIO::init_terminal_io(Word reset_key)
{
    struct termios  buf;
    tcflag_t    mask;
    struct sigaction sig_action;
    struct sigaction old_action;

    memset(&sig_action, 0, sizeof(sig_action));

    if (isatty(fileno(stdin)))
    {
        if (tcgetattr(fileno(stdin), &save_termios) < 0)
        {
            fprintf(stderr, "unable to initialize terminal\n");

            scheduler.request_new_state(CpuState::Exit);
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

            // Add some signal handlers
            sig_action.sa_sigaction = s_exec_signal;
            sig_action.sa_flags = SA_RESTART | SA_SIGINFO;
            sigemptyset(&sig_action.sa_mask);

#if defined(SIGUSR1)
            sigaction(SIGUSR1, &sig_action, &old_action);
#endif
#if defined(SIGUSR2)
            sigaction(SIGUSR2, &sig_action, &old_action);
#endif
#if defined(VINTR) && defined(SIGINT)
            buf.c_cc[VINTR] = reset_key;
            sigaction(SIGINT, &sig_action, &old_action);
#endif
#if defined(VQUIT) && defined(SIGQUIT)
            buf.c_cc[VQUIT] = disable;
            sigaction(SIGQUIT, &sig_action, &old_action);
#endif
#if defined(VQUIT) && defined(SIGTERM)
            buf.c_cc[VQUIT] = disable;
            sigaction(SIGTERM, &sig_action, &old_action);
#endif
#ifdef VSUSP
            buf.c_cc[VSUSP] = disable;
#endif
#if defined(VSUSP) && defined(SIGQUIT)
            buf.c_cc[VSUSP] = disable;
#ifdef VDSUSP
            buf.c_cc[VDSUSP] = disable;
#endif
            sigaction(SIGTSTP, &sig_action, &old_action);
#endif
        }

        if (tcsetattr(fileno(stdin), TCSAFLUSH, &buf) < 0)
        {
            // on error try to switch back,
            // otherwise terminal is damaged
            tcsetattr(fileno(stdin), TCSAFLUSH, &save_termios);
            fprintf(stderr, "unable to initialize terminal\n");

            scheduler.request_new_state(CpuState::Exit);

            return;
        }

        // use atexit here to reset the Terminal IO
        // because the X11 interface under some error
        // condition like missing DISPLAY variable or
        // X11 protocol error aborts with exit()
        atexit(reset_terminal_io);
    }
}
#else
void TerminalIO::init_terminal_io(Word /* [[maybe_unused]]  Word reset_key */)
{
#if defined(_WIN32) && defined(SIGTERM)
    signal(SIGTERM, s_exec_signal);
#endif
}
#endif // #ifdef HAVE_TERMIOS_H

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
    // After a reset delay the serial key input request.
    // Reason: After output one line FLEX requests for keyboard input.
    // If startup command is present any keyboard input has to be
    // delayed until the FLEX prompt.
    if (init_delay)
    {
        --init_delay;
        return false;
    }

#ifdef HAVE_TERMIOS_H
    static Word count = 0;

    if (++count >= 100)
    {
        char buffer[1] = { '\0' };
        count = 0;
        fflush(stdout);

        if (read(fileno(stdin), &buffer, 1) > 0)
        {
            put_char_serial(buffer[0]);
        }
    }
#endif // #ifdef HAVE_TERMIOS_H

    std::lock_guard<std::mutex> guard(serial_mutex);
    return !key_buffer_serial.empty();
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

void TerminalIO::write_char_serial_safe(Byte value)
{
    (void)value;
#ifdef HAVE_TERMIOS_H
    // the write syscall may be aborted by EINTR or no byte is written.
    // This is defined bahaviour.
    // Solution: Retry up to 4 times.
    for (int i = 0; i < 4; ++i)
    {
        if (write(fileno(stdout), &value, 1) == 1)
        {
            break;
        }
    }
#endif
}

#ifdef HAVE_TERMIOS_H
void TerminalIO::write_char_serial(Byte value)
{
    used_serial_io = true;
#ifdef VERASE

    if (value == BACK_SPACE)
    {
        write_char_serial_safe('\b');
        write_char_serial_safe(' ');
        write_char_serial_safe('\b');
    }
    else
#endif
    // NUL characters are intentionally ignored.
    // Their initial usage to wait until the teleprinter returns to the first
    // printing position is not needed any more. There are terminals which
    // incorrectly display is as a space.
    // For details see: https://en.wikipedia.org/wiki/Null_character
    // Also ESC characters are intentionally ignored.
    if (value != '\0' && value != '\x1b')
    {
        write_char_serial_safe(value);
    }
}
#else
void TerminalIO::write_char_serial(Byte /* [[maybe_unused]] Byte value */)
{
}
#endif // #ifdef HAVE_TERMIOS_H

bool TerminalIO::is_terminal_supported()
{
#ifdef HAVE_TERMIOS_H
    return true;
#else
    return false;
#endif
}

#ifdef _WIN32
void TerminalIO::s_exec_signal(int sig_no)
{
    if (TerminalIO::instance != nullptr)
    {
        TerminalIO::instance->exec_signal(sig_no);
    }
}
#else
void TerminalIO::s_exec_signal(int sig_no, siginfo_t *, void *)
{
    if (TerminalIO::instance != nullptr)
    {
        TerminalIO::instance->exec_signal(sig_no);
    }
}
#endif

void TerminalIO::exec_signal(int sig_no)
{
    switch (sig_no)
    {
        case SIGINT:
            Notify(NotifyId::SetNmi);
            break;

#if defined(SIGUSR1)
        case SIGUSR1:
            Notify(NotifyId::SetIrq);
            break;
#endif

#if defined(SIGUSR2)
        case SIGUSR2:
            Notify(NotifyId::SetFirq);
            break;
#endif

#if defined(SIGQUIT)
        case SIGQUIT:
            scheduler.request_new_state(CpuState::Exit);
            break;
#endif
#if defined(SIGTERM)
        case SIGTERM:
            scheduler.request_new_state(CpuState::Exit);
            break;
#endif

#if defined(SIGTSTP)
        case SIGTSTP:
            scheduler.request_new_state(CpuState::ResetRun);
            break;
#endif
    }
}

void TerminalIO::set_startup_command(const char *x_startup_command)
{
    std::string startup_command(x_startup_command);

    if (!startup_command.empty())
    {
        std::lock_guard<std::mutex> guard(serial_mutex);
        std::copy(startup_command.begin(), startup_command.end(),
                  std::back_inserter(key_buffer_serial));
        key_buffer_serial.push_back('\r');
    }
}
