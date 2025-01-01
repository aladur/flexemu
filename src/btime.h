/*
    btime.h

    Basic class containing a time with resolution seconds.
    Copyright (C) 2022-2025  W. Schwotzer

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

#ifndef BTIME_INCLUDED
#define BTIME_INCLUDED

#include <cstdint>
#include <string>

class BTime
{
private:

    int hour{0}; // range 0..23
    int minute{0}; // range 0..59
    int second{0}; // range 0..59

public:

    enum class Format : uint8_t
    {
        HHMMSS, // HH:MM:SS hour:minute:second
        HHMM, // HH:MM hour:minute
    };

    explicit BTime() = default;
    BTime(int h, int m, int s = 0);
    BTime(const BTime &src) = default;
    ~BTime() = default;

    static BTime Now();
    std::string AsString(Format format = Format::HHMMSS) const;
    void Get(int &hour, int &minute, int &second) const;
    void Set(int hour, int minute, int second);
    void Set(const BTime &src);
    int GetHour() const;
    int GetMinute() const;
    int GetSecond() const;
    size_t ToSeconds() const;
    BTime &operator = (const BTime &src) = default;
};

bool operator == (const BTime &rhs, const BTime &lhs);
bool operator != (const BTime &rhs, const BTime &lhs);
bool operator < (const BTime &rhs, const BTime &lhs);
bool operator > (const BTime &rhs, const BTime &lhs);
bool operator <= (const BTime &rhs, const BTime &lhs);
bool operator >= (const BTime &rhs, const BTime &lhs);
#endif

