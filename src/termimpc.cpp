/*
    termimpc.cpp


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


#include "e2.h"
#include "misc1.h"
#include "soptions.h"
#include "termimpc.h"
#include "asciictl.h"
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <csignal>
#include <unordered_map>
#ifdef UNIX
#include "config.h"
#include <unistd.h>
#endif

#ifdef UNIX
WINDOW *NCursesTerminalImpl::win = nullptr;
#endif

NCursesTerminalImpl::NCursesTerminalImpl(const struct sOptions &p_options)
    : options(p_options)
{
}

// Return false on fatal errors forcing to abort the application.
bool NCursesTerminalImpl::init(Word reset_key, fct_sigaction fct)
{
    (void)fct;
    const auto success = init_terminal_io(reset_key);

#ifdef UNIX
    if (success)
    {
        struct sigaction sig_action{};

        sig_action.sa_sigaction = fct;
        sig_action.sa_flags = SA_RESTART | SA_SIGINFO;
        sigemptyset(&sig_action.sa_mask);
        sigaction(SIGUSR1, &sig_action, nullptr);
        sigaction(SIGUSR2, &sig_action, nullptr);
        sigaction(SIGINT, &sig_action, nullptr);
        sigaction(SIGQUIT, &sig_action, nullptr);
        sigaction(SIGTERM, &sig_action, nullptr);
        sigaction(SIGTSTP, &sig_action, nullptr);
    }
#endif

    return success;
}

void NCursesTerminalImpl::reset_serial()
{
    init_delay = 500;
    input_delay = 0;
    is_cursor_visible = true;
    is_german = false;
#ifdef UNIX
    if (win != nullptr)
    {
        wclear(win);
    }
#endif

    std::lock_guard<std::mutex> guard(serial_mutex);
    key_buffer_serial.clear();
}

void NCursesTerminalImpl::reset_terminal_io()
{
#ifdef UNIX
    if (win != nullptr)
    {
        delwin(win);
        win = nullptr;
        endwin();
    }
#endif
}

bool NCursesTerminalImpl::init_terminal_io(Word reset_key) const
{
    (void)reset_key;

#ifdef UNIX
    setlocale(LC_CTYPE, "");
    // Before calling initscr() isatty() has to make sure that stdin is
    // a file descriptor referring to a terminal. If this is not the case
    // initscr() would abort the application.
    if (isatty(fileno(stdin)) == 1 && initscr() != nullptr)
    {
        raw();
        noecho();
        nonl();
        curs_set(is_cursor_visible);
        win = newwin(TERM_LINES, TERM_COLUMNS, 0, 0);
        keypad(win, TRUE);
        nodelay(win, TRUE);
    }

    return true;
#endif
#ifdef _WIN32
    return false;
#endif
}

// poll serial port for input character.
bool NCursesTerminalImpl::has_char_serial()
{
#ifdef UNIX
    // After a reset and booting FLEX delay the serial key input request.
    // Reason: After output one line FLEX requests for keyboard input.
    // If startup command is present any keyboard input has to be
    // delayed until the FLEX prompt.
    if (init_delay)
    {
        --init_delay;
        return false;
    }

    static Word count = 0;

    if (++count >= 100)
    {
        count = 0;

        chtype buffer = wgetch(win);
        if (buffer != static_cast<chtype>(ERR))
        {
            Byte key{};

            if (convert_to_pat09_key(buffer, key))
            {
                put_char_serial(key);
            }
        }
    }

    // After successfully receiving a character from terminal delay
    // signaling the next characters being receivable.
    if (input_delay != 0)
    {
        --input_delay;
        return false;
    }

    std::lock_guard<std::mutex> guard(serial_mutex);
    return !key_buffer_serial.empty();
#endif
#ifdef _WIN32
    return false;
#endif
}

// Read a serial character from cpu.
// ATTENTION: Input should always be polled before read_char_serial.
Byte NCursesTerminalImpl::read_char_serial()
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
Byte NCursesTerminalImpl::peek_char_serial()
{
    Byte result = 0x00;

    std::lock_guard<std::mutex> guard(serial_mutex);
    if (!key_buffer_serial.empty())
    {
        result = key_buffer_serial.front();
    }

    return result;
}

void NCursesTerminalImpl::write_char_serial(Byte value)
{
    (void)value;
#ifdef UNIX
    // NUL characters are optionally ignored.
    // Their initial usage to wait until the teleprinter returns to the first
    // printing position is not needed any more. There are terminals which
    // incorrectly display it as a space.
    // For details see: https://en.wikipedia.org/wiki/Null_character
    if (value != '\0' || !options.isTerminalIgnoreNUL)
    {
        write_char_serial_safe(value);
    }
#endif
}

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
bool NCursesTerminalImpl::is_terminal_supported()
{
#ifdef UNIX
    return win != nullptr;
#endif
#ifdef _WIN32
    return false;
#endif
}

void NCursesTerminalImpl::set_startup_command(const std::string &startup_command)
{
    if (!startup_command.empty())
    {
        std::lock_guard<std::mutex> guard(serial_mutex);
        std::copy(startup_command.begin(), startup_command.end(),
                  std::back_inserter(key_buffer_serial));
        key_buffer_serial.push_back('\r');
    }
}

#ifdef UNIX
void NCursesTerminalImpl::put_char_serial(Byte key)
{
    std::lock_guard<std::mutex> guard(serial_mutex);

    key_buffer_serial.push_back(key);
}

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void NCursesTerminalImpl::write_char_serial_safe(Byte value)
{
    bool char_written = false;

    // For Eurocom II terminal compatibilty mask output character to 7-bit.
    if ((value & 0x80U) != 0U)
    {
        value &= 0x7FU;
    }

    if (!esc_sequence.empty())
    {
        process_esc_sequence(value);
        return;
    }

    if (value < 0x20)
    {
        process_ctrl_character(value);
        return;
    }

#ifdef HAVE_NCURSESW
    if (is_german)
    {
        static const std::string ascii{"{|}[\\]~"};
        // utf-8 encoding of german umlauts
        constexpr const std::array<const char *, 7> umlaut{
            "\xC3\xA4", // ae
            "\xC3\xB6", // oe
            "\xC3\xBC", // ue
            "\xC3\x84", // AE
            "\xC3\x96", // OE
            "\xC3\x9C", // UE
            "\xC3\x9F"}; // ss

        assert(ascii.size() == umlaut.size());
        const auto pos = ascii.find(static_cast<char>(value));
        if (pos != std::string::npos)
        {
            waddstr(win, umlaut[pos]);
            char_written = true;
        }
    }
#endif

    if (!char_written)
    {
        if (value == 0x7FU)
        {
            constexpr const char *south_west_arrow{"\xE2\x86\x99"};
            waddstr(win, south_west_arrow);
        }
        else
        {
            waddch(win, value);
        }
    }

    int x;
    int y;

    getyx(win, y, x);
    if (x >= TERM_COLUMNS - 1)
    {
        if (y >= TERM_LINES - 1)
        {
            wmove(win, 0, x);
            wdeleteln(win);
            wmove(win, y, 0);
        }
        else
        {
            wmove(win, y + 1, 0);
        }
    }
    wrefresh(win);
}

void NCursesTerminalImpl::limit_column_and_line(int &column, int &line)
{
    if (column < 0 || column >= TERM_COLUMNS)
    {
        column = getcurx(win);
    }

    if (line < 0 || line >= TERM_LINES)
    {
        line = getcury(win);
    }
}

void NCursesTerminalImpl::process_ctrl_character(Byte value)
{
    int x;
    int y;

    switch (value)
    {
        case '\x5': // CTRL-E: switch off cursor
            is_cursor_visible = false;
            curs_set(is_cursor_visible);
            break;

        case BEL: // CTRL-G: Ring bell.
            beep();
            break;

        case BS: // CTRL-H: back space
            getyx(win, y, x);
            if (x > 0)
            {
                wmove(win, y, x - 1);
                waddch(win, ' ');
                wmove(win, y, x - 1);
                wrefresh(win);
            }
            break;

        case FF: // CTRL-L: Form feed
            wclear(win);
            [[fallthrough]];
        case '\x1C': // CTRL-\ cursor home
            wmove(win, 0, 0);
            wrefresh(win);
            break;

        case '\x0E': // CTRL-N: scroll up one line
            getyx(win, y, x);
            wmove(win, 0, x);
            wdeleteln(win);
            wmove(win, y, x);
            wrefresh(win);
            break;

        case '\x0F': // CTRL-O: scroll down one line
            getyx(win, y, x);
            wmove(win, 0, x);
            winsertln(win);
            wmove(win, y, x);
            wrefresh(win);
            break;

        case '\x11': // CTRL-Q: cursor up one line
            getyx(win, y, x);
            wmove(win, std::max(y - 1, 0), x);
            break;

        case '\x12': // CTRL-R: cursor down one line
            getyx(win, y, x);
            wmove(win, std::min(y + 1, TERM_LINES - 1), x);
            break;

        case '\x13': // CTRL-S: cursor right one line
            getyx(win, y, x);
            wmove(win, y, std::min(x + 1, TERM_COLUMNS - 1));
            break;

        case '\x14': // CTRL-T: cursor left one line
            getyx(win, y, x);
            wmove(win, y, std::max(x - 1, 0));
            break;

        case '\x15': // CTRL-U: switch on cursor
            is_cursor_visible = true;
            curs_set(is_cursor_visible);
            break;

        case ESC: // Start ESC sequence.
            esc_sequence.clear();
            esc_sequence.push_back(ESC);
            curs_set(FALSE);
            break;

        case '\x1D': // CTRL-]: toggle soft/hard scroll, not supported
            break;

        case LF:
            getyx(win, y, x);
            if (y == TERM_LINES - 1)
            {
                wmove(win, 0, x);
                wdeleteln(win);
                wmove(win, y, 0);
                wrefresh(win);
            }
            else
            {
                wmove(win, y + 1, x);
            }
            break;

        case CR:
            wmove(win, getcury(win), 0);
            break;
    }
}

void NCursesTerminalImpl::process_esc_sequence(Byte value)
{
    const static std::string esc_chars("BCDFGIPpQqKLZkS");

    esc_sequence.push_back(value);

    if (esc_sequence.size() == 2 &&
        value != 'A' && value != 'E' && value != '=')
    {
        if (esc_chars.find(static_cast<char>(value)) == std::string::npos)
        {
            // Unsupported ESC sequence, abort.
            esc_sequence.clear();
            curs_set(is_cursor_visible);
            return;
        }

        // Process ESC sequences with two characters.
        switch (value)
        {
            case 'B': // ESC B: delete line
                wdeleteln(win);
                break;

            case 'C': // ESC C: insert line
                winsertln(win);
                break;

            case 'P': // ESC P, ESC p: Switch on reverse char display
            case 'p':
                wattron(win, A_REVERSE);
                break;

            case 'Q': // ESC Q, ESC q: Switch off reverse char display
            case 'q':
                wattroff(win, A_REVERSE);
                break;

            case 'K': // ESC K, ESC L: Erase to end of line
            case 'L':
                wclrtoeol(win);
                break;

            case 'Z': // ESC Z, ESC k, ESC S: Erase to end of screen
            case 'k':
            case 'S':
                wclrtobot(win);
                break;

            case 'D': // ESC D: switch to german char. set
                is_german = true;
                break;

            case 'I': // ESC I: switch to ASCII char. set
                is_german = false;
                break;

            case 'F': // ESC F: switch to insert cursor, not supported.
            case 'G': // ESC G: switch to overwrite cursor, not supported.
            default:
                break;
        }
        esc_sequence.clear();
        curs_set(is_cursor_visible);
        wrefresh(win);
        return;
    }

    // process ESC sequences with more than two characters.
    if ((esc_sequence[1] == 'A' && esc_sequence.size() < 6) ||
         esc_sequence.size() < 4)
    {
        return;
    }

    int line;
    int column;

    switch (esc_sequence[1])
    {
        case 'A': // ESC A: plot a dot at XX XX YY YY, not supported
            break;

        case 'E': // ESC E: move cursor to YY XX
            line = esc_sequence[2] - 32;
            column = esc_sequence[3] - 32;
            limit_column_and_line(column, line);
            wmove(win, line, column);
            break;

        case '=': // ESC =: Move cursor to XX YY
            line = esc_sequence[3] - 32;
            column = esc_sequence[2] - 32;
            limit_column_and_line(column, line);
            wmove(win, line, column);
            break;
    }

    esc_sequence.clear();
    curs_set(is_cursor_visible);
    wrefresh(win);
}

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
bool NCursesTerminalImpl::convert_to_pat09_key(chtype buffer, Byte &key) const
{
    static const std::unordered_map<chtype, Byte> key_from_code{
        // Ente and editing keys
        { KEY_BACKSPACE, '\x08' }, // Backspace
        { KEY_DC, '\x7F' }, // Delete
        { KEY_SDC, '\x7F' }, // Shift+Delete
        { KEY_ENTER, '\x0D' },
        // Navigation keys
        { KEY_IC, '\xF2' }, // Insert
        { KEY_HOME, '\xF1' }, // Home
        { KEY_UP, '\xF8' }, // Up
        { KEY_PPAGE, '\xF7' }, // PageUp
        { KEY_LEFT, '\xF4' }, // Left
        { KEY_BEG, '\xF5' }, // Begin
        { KEY_RIGHT, '\xF6' }, // Right
        { KEY_END, '\xF9' }, // End
        { KEY_DOWN, '\xF2' }, // Down
        { KEY_NPAGE, '\xF3' }, // PageDown
        // Navigation keys with Shift
        { KEY_SIC, '\xE2' }, // Shift+Insert
        { KEY_SHOME, '\xE1' }, // Shift+Home
        { KEY_SR, '\xE8' }, // Shift+Up
        { KEY_SPREVIOUS, '\xE7' }, // Shift+PageUp
        { KEY_SLEFT, '\xE4' }, // Shift+Left
        { KEY_SRIGHT, '\xE6' }, // Shift+Right
        { KEY_SEND, '\xE9' }, // Shift+End
        { KEY_SF, '\xE2' }, // Shift+Down
        { KEY_SNEXT, '\xE3' }, // Shift+PageDown
        // Function keys
        { KEY_F(1), '\xC0' },
        { KEY_F(2), '\xC1' },
        { KEY_F(3), '\xC2' },
        { KEY_F(4), '\xC3' },
        { KEY_F(5), '\xC4' },
        { KEY_F(6), '\xC5' },
        { KEY_F(7), '\xC6' },
        { KEY_F(8), '\xC7' },
        { KEY_F(9), '\xC8' },
        { KEY_F(10), '\xC9' },
        { KEY_F(13), '\xCA' },
        { KEY_F(14), '\xCB' },
        { KEY_F(15), '\xCC' },
        { KEY_F(16), '\xCD' },
        { KEY_F(17), '\xCE' },
        { KEY_F(18), '\xCF' },
        { KEY_F(19), '\xD0' },
        { KEY_F(20), '\xD1' },
        { KEY_F(21), '\xD2' },
    };
    static const std::unordered_map<std::string, Byte> key_from_name{
        // Navigation keys
        { "kHOM5", '\xB1' }, // Ctrl+Home
        { "kEND5", '\xB9' }, // Ctrl+End
        { "kPRV5", '\xB7' }, // Ctrl+PageUp
        { "kNXT5", '\xB3' }, // Ctrl+PageDown
        { "kHOM6", '\xA1' }, // Shift+Ctrl+Home
        { "kEND6", '\xA9' }, // Shift+Ctrl+End
        { "kPRV6", '\xA7' }, // Shift+Ctrl+PageUp
        { "kNXT6", '\xA3' }, // Shift+Ctrl+PageDown
        // Cursor navigation keys
        { "kUP5", '\xB8' }, // Ctrl+Up
        { "kDN5", '\xB2' }, // Ctrl+Down
        { "kLFT5", '\xB4' }, // Ctrl+Left
        { "kRIT5", '\xB6' }, // Ctrl+Right
        { "kUP6", '\xA8' }, // Shift+Ctrl+Up
        { "kDN6", '\xA2' }, // Shift+Ctrl+Down
        { "kLFT6", '\xA4' }, // Shift+Ctrl+Left
        { "kRIT6", '\xA6' }, // Shift+Ctrl+Right
        // Edit keys (Insert does not work)
        { "kDC5", '\x1F' }, // Ctrl+Delete
        { "kDC6", '\x7F' }, // Shift+Ctrl+Delete
        // Numeric keypad
        { "kpADD", '\x2B' }, // Addition
        { "kpSUB", '\x2D' }, // Subtraction
        { "kpMUL", '\x2A' }, // Multiplication
        { "kpDIV", '\x2F' }, // Division
    };

    if (buffer <= '~')
    {
        key = static_cast<Byte>(buffer);
        return true;
    }

    const auto iter_code = key_from_code.find(buffer);
    if (iter_code != key_from_code.end())
    {
        key = iter_code->second;
        return true;
    }

    const auto name = std::string(keyname(static_cast<int>(buffer)));
    const auto iter_name = key_from_name.find(name);
    if (iter_name != key_from_name.end())
    {
        key = iter_name->second;
        return true;
    }

    return false;
}
#endif
