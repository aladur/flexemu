/*
    keyboard.h


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



#ifndef KEYBOARD_INCLUDED
#define KEYBOARD_INCLUDED

#include "misc1.h"
#include "flexemu.h"
#include "asciictl.h"
#include <string>
#include <deque>
#include <mutex>
#include <memory>
#include <optional>

// key mask for shift, control key
enum : uint8_t {
SHIFT_KEY = 8,
CONTROL_KEY = 16,
};

class KeyboardIO
{
    std::mutex parallel_mutex;
    std::deque<Byte> key_buffer_parallel;
    unsigned int keyMask{0};
    Word init_delay{500};
    std::optional<Byte> optional_boot_char;

public:
    static void set_bell(Word p_percent);
    void reset_parallel();
    bool has_key_parallel(bool &do_notify);
    Byte read_char_parallel(bool &do_notify);
    Byte peek_char_parallel();
    void put_char_parallel(Byte key, bool &do_notify);
    void put_value(unsigned int keyMask);
    void get_value(unsigned int *keyMask) const;
    void set_startup_command(const std::string &startup_command);
    void set_boot_char(const std::optional<Byte> &p_optional_boot_char);

    KeyboardIO();
};

#endif // KEYBOARD_INCLUDED

