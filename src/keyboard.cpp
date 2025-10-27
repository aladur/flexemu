/*
    keyboard.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

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


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#if defined(UNIX) || defined(USE_CMAKE)
#include "config.h"
#else
#include "confignt.h"
#endif
#include "typedefs.h"
#include "keyboard.h"
#include "asciictl.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mutex>
#include <optional>
#include <iterator>
#include <string>
#include <algorithm>


KeyboardIO::KeyboardIO()
{
    reset_parallel();
}

void KeyboardIO::reset_parallel()
{
    init_delay = 500;
    optional_boot_char.reset();
    std::lock_guard<std::mutex> guard(parallel_mutex);
    key_buffer_parallel.clear();
}

void KeyboardIO::put_char_parallel(Byte key, bool &do_notify)
{
    do_notify = false;

    std::lock_guard<std::mutex> guard(parallel_mutex);
    bool was_empty = key_buffer_parallel.empty();
    key_buffer_parallel.push_back(key);
    if (was_empty)
    {
        do_notify = true;
    }
}

bool KeyboardIO::has_key_parallel(bool &do_notify)
{
    do_notify = false;

    if (optional_boot_char.has_value())
    {
        do_notify = true;
        return true;
    }

    // After a reset and booting FLEX delay the parallel key input request.
    // Reason: After output one line FLEX requests for keyboard input.
    // If startup command is present any keyboard input has to be
    // delayed until the FLEX prompt.
    if (init_delay)
    {
        --init_delay;
        if (!init_delay)
        {
            std::lock_guard<std::mutex> guard(parallel_mutex);
            if (!key_buffer_parallel.empty())
            {
                do_notify = true;
            }
        }
        return do_notify;
    }

    std::lock_guard<std::mutex> guard(parallel_mutex);
    return !key_buffer_parallel.empty();
}

// Read character and remove it from the queue.
// Input should always be polled before read_char_parallel.
Byte KeyboardIO::read_char_parallel(bool &do_notify)
{
    Byte result = 0x00;

    do_notify = false;

    if (optional_boot_char.has_value())
    {
        result = optional_boot_char.value();
        optional_boot_char.reset();
        std::lock_guard<std::mutex> guard(parallel_mutex);
        if (!key_buffer_parallel.empty())
        {
            do_notify = true;
        }
        return result;
    }

    std::lock_guard<std::mutex> guard(parallel_mutex);
    if (!key_buffer_parallel.empty())
    {
        result = key_buffer_parallel.front();
        key_buffer_parallel.pop_front();

        // If there are still characters in the
        // buffer set CA1 flag again.
        if (!key_buffer_parallel.empty())
        {
            do_notify = true;
        }
    }

    return result;
}

// Read character, but leave it in the queue.
// Input should always be polled before read_queued_ch.
Byte KeyboardIO::peek_char_parallel()
{
    Byte result = 0x00;

    std::lock_guard<std::mutex> guard(parallel_mutex);
    if (!key_buffer_parallel.empty())
    {
        result = key_buffer_parallel.front();
    }

    return result;
}

void KeyboardIO::set_bell(Word /*p_percent*/)
{
#ifdef _WIN32
    Beep(400, 100);
#endif
#ifdef UNIX
    // the write syscall may be aborted by EINTR or no byte is written.
    // This is defined bahaviour.
    // Solution: Retry up to 4 times.
    constexpr const char bell = BEL;

    for (int i = 0; i < 4; ++i)
    {
        if (write(fileno(stdout), &bell, 1) == 1)
        {
            break;
        }
    }
#endif
}

void KeyboardIO::put_value(unsigned int p_keyMask)
{
    keyMask = p_keyMask;
}

void KeyboardIO::get_value(unsigned int *p_keyMask) const
{
    if (p_keyMask != nullptr)
    {
        *p_keyMask = keyMask;
    }
}

void KeyboardIO::set_startup_command(const std::string &startup_command)
{
    if (!startup_command.empty())
    {
        std::lock_guard<std::mutex> guard(parallel_mutex);
        std::copy(startup_command.begin(), startup_command.end(),
                  std::back_inserter(key_buffer_parallel));
        key_buffer_parallel.push_back('\r');
    }
}

void KeyboardIO::set_boot_char(const std::optional<Byte> &p_optional_boot_char)
{
    optional_boot_char = p_optional_boot_char;
}

