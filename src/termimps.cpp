/*
    termimps.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2025  W. Schwotzer

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


#include "config.h"
#include "typedefs.h"
#include "termimpi.h"
#include "bobshelp.h"
#include "termimps.h"
#include "soptions.h"
#include "asciictl.h"
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <csignal>
#include <cstring>
#include <cstdio>
#include <mutex>
#include <string>
#include <iostream>
#include <iterator>
#include <algorithm>


ScrollingTerminalImpl::ScrollingTerminalImpl(const sOptions &p_options)
    : options(p_options)
    , init_delay(500)
{
}

// Return false on fatal errors forcing to abort the application.
bool ScrollingTerminalImpl::init(Word reset_key, fct_sigaction fct)
{
    (void)reset_key;
    (void)fct;
#ifdef HAVE_TERMIOS_H
    struct termios buf{};
    tcflag_t mask;
    struct sigaction sig_action{};
    struct sigaction old_action{};

    std::memset(&sig_action, 0, sizeof(sig_action));

    if (isatty(fileno(stdin)))
    {
        if (tcgetattr(fileno(stdin), &buf) < 0)
        {
            return false;
        }

        if (!is_termios_saved)
        {
            save_termios = buf;
        }

        // c_lflag:
        mask = 0U
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
        mask = 0U
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
        buf.c_cc[VMIN] = 0;
        buf.c_cc[VTIME] = 0;
        long disable = fpathconf(fileno(stdin), _PC_VDISABLE);

        if (disable < 0)
        {
            disable = reset_key;
        }

        // Add some signal handlers
        sig_action.sa_sigaction = fct;
        sig_action.sa_flags = SA_RESTART | SA_SIGINFO;
        sigemptyset(&sig_action.sa_mask);

        sigaction(SIGUSR1, &sig_action, &old_action);
        sigaction(SIGUSR2, &sig_action, &old_action);
#if defined(VINTR)
        buf.c_cc[VINTR] = reset_key;
        sigaction(SIGINT, &sig_action, &old_action);
#endif
#if defined(VQUIT)
        buf.c_cc[VQUIT] = disable;
        sigaction(SIGQUIT, &sig_action, &old_action);
#endif
#if defined(VQUIT)
        buf.c_cc[VQUIT] = disable;
        sigaction(SIGTERM, &sig_action, &old_action);
#endif
#ifdef VSUSP
        buf.c_cc[VSUSP] = disable;
#ifdef VDSUSP
        buf.c_cc[VDSUSP] = disable;
#endif
        sigaction(SIGTSTP, &sig_action, &old_action);
#endif

        if (tcsetattr(fileno(stdin), TCSAFLUSH, &buf) < 0)
        {
            // on error try to switch back to previous termios settings,
            // otherwise terminal is unusable.
            tcsetattr(fileno(stdin), TCSAFLUSH, &save_termios);
            return false;
        }

        is_termios_saved = true;
    }

    return true;
#endif // #ifdef HAVE_TERMIOS_H

    return false;
}

void ScrollingTerminalImpl::reset_serial()
{
    init_delay = 500;
    was_escape = false;
    std::lock_guard<std::mutex> guard(serial_mutex);
    key_buffer_serial.clear();
}

// poll serial port for input character.
bool ScrollingTerminalImpl::has_char_serial()
{
    // After a reset and booting FLEX delay the serial key input request.
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
        Byte buffer{};
        count = 0;
        fflush(stdout);

        if (read(fileno(stdin), &buffer, 1) > 0)
        {
            put_char_serial(buffer);
        }
    }

    // After successfully receiving a character from terminal delay
    // signaling the next characters being receivable.
    if (input_delay != 0)
    {
        --input_delay;
        return false;
    }
#endif // #ifdef HAVE_TERMIOS_H

    std::lock_guard<std::mutex> guard(serial_mutex);
    return !key_buffer_serial.empty();
}

// Read a serial character from cpu.
// ATTENTION: Input should always be polled before read_char_serial.
Byte ScrollingTerminalImpl::read_char_serial()
{
    Byte result = 0x00;

    std::lock_guard<std::mutex> guard(serial_mutex);
    if (!key_buffer_serial.empty())
    {
        result = key_buffer_serial.front();
        key_buffer_serial.pop_front();
        // After successfully receiving a character from terminal delay
        // signalling the next character being receivable.
        // Reason: Immediately receiving a second character may get lost,
        // resulting in receiving only every second character of the
        // startup command (-C).
        input_delay = 2;
    }

    return result;
}

// Read character, but leave it in the queue.
// ATTENTION: Input should always be polled before peek_char_serial.
Byte ScrollingTerminalImpl::peek_char_serial()
{
    Byte result = 0x00;

    std::lock_guard<std::mutex> guard(serial_mutex);
    if (!key_buffer_serial.empty())
    {
        result = key_buffer_serial.front();
    }

    return result;
}

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void ScrollingTerminalImpl::write_char_serial_safe(Byte value)
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

void ScrollingTerminalImpl::write_char_serial(Byte value)
{
    (void)value;
#ifdef HAVE_TERMIOS_H
    used_serial_io = true;
#ifdef VERASE

    if (value == BS)
    {
        write_char_serial_safe('\b');
        write_char_serial_safe(' ');
        write_char_serial_safe('\b');
    }
    else
#endif
    // NUL characters are optionally ignored.
    // Their initial usage to wait until the teleprinter returns to the first
    // printing position is not needed any more. There are terminals which
    // incorrectly display it as a space.
    // For details see: https://en.wikipedia.org/wiki/Null_character
    // Also ESC characters are optionally ignored.
    // When ignoreing ESC also one character after ESC is ignored.
    if ((value != NUL || !options.isTerminalIgnoreNUL) &&
        (value != ESC || !options.isTerminalIgnoreESC) &&
        (!was_escape || !options.isTerminalIgnoreESC))
    {
        write_char_serial_safe(value);
    }

    if (options.isTerminalIgnoreESC)
    {
        was_escape = (!was_escape && (value == ESC));
    }
#endif // #ifdef HAVE_TERMIOS_H
}

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
bool ScrollingTerminalImpl::is_terminal_supported()
{
#ifdef HAVE_TERMIOS_H
    return is_termios_saved;
#else
    return false;
#endif
}

void ScrollingTerminalImpl::set_startup_command(
        const std::string &startup_command)
{
    if (!startup_command.empty())
    {
        std::lock_guard<std::mutex> guard(serial_mutex);
        std::copy(startup_command.begin(), startup_command.end(),
                  std::back_inserter(key_buffer_serial));
        key_buffer_serial.push_back('\r');
    }
}

void ScrollingTerminalImpl::reset_terminal_io()
{
#ifdef HAVE_TERMIOS_H
    if (is_termios_saved && isatty(fileno(stdin)))
    {
        tcsetattr(0, TCSAFLUSH, &save_termios);

        if (used_serial_io)
        {
            std::cout << "\n";
        }
    }
#endif // #ifdef HAVE_TERMIOS_H
}

void ScrollingTerminalImpl::put_char_serial(Byte key)
{
    (void)key;

    {
        std::lock_guard<std::mutex> guard(serial_mutex);
        // convert back space character
#ifdef HAVE_TERMIOS_H
#ifdef VERASE

        if (key == save_termios.c_cc[VERASE] || key == 0x7f)
        {
            key = BS;
        }

#endif
#endif // #ifdef HAVE_TERMIOS_H

        key_buffer_serial.push_back(key);
    }

    Notify(NotifyId::KeyPressedOnCPU, &key);
}
