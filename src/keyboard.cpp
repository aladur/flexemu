/*
    inout.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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
#include "keyboard.h"
#include <iterator>


KeyboardIO::KeyboardIO()
{
    reset_parallel();
}

void KeyboardIO::reset_parallel()
{
    init_delay = 500;
    std::lock_guard<std::mutex> guard(parallel_mutex);
    key_buffer_parallel.clear();
}

void KeyboardIO::put_char_parallel(Byte key, bool &do_notify)
{
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
    // After a reset delay the parallel key input request.
    // Reason: After output one line FLEX requests for keyboard input.
    // If startup command is present any keyboard input has to be
    // delayed until the FLEX prompt.
    if (init_delay)
    {
        --init_delay;
        if (!init_delay)
        {
            do_notify = true;
        }
        return false;
    }
    std::lock_guard<std::mutex> guard(parallel_mutex);
    return !key_buffer_parallel.empty();
}

// Read character and remove it from the queue.
// Input should always be polled before read_char_parallel.
Byte KeyboardIO::read_char_parallel(bool &do_notify)
{
    Byte result = 0x00;

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
    static char bell = BEL;

    write(fileno(stdout), &bell, 1);
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

void KeyboardIO::set_startup_command(const char *p_startup_command)
{
    std::string startup_command(p_startup_command);

    if (!startup_command.empty())
    {
        std::lock_guard<std::mutex> guard(parallel_mutex);
        std::copy(startup_command.begin(), startup_command.end(),
                  std::back_inserter(key_buffer_parallel));
        key_buffer_parallel.push_back('\r');
    }
}

