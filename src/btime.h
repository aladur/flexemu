/*
    btime.h

    Basic class containing a time with resolution seconds.
    Copyright (C) 2022  W. Schwotzer

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

#include <string>

class BTime
{
private:

    uint8_t hour; // range 0..23
    uint8_t minute; // range 0..59
    uint8_t second; // range 0..59

public:

    BTime(uint8_t h = 0U, uint8_t m = 0U, uint8_t s = 0U);
    BTime(const BTime &src);
    ~BTime();

    static BTime Now();
    const std::string AsString() const;
    void Get(uint8_t &hour, uint8_t &minute, uint8_t &second) const;
    void Set(uint8_t hour, uint8_t minute, uint8_t second);
    void Set(const BTime &src);
    uint8_t GetHour() const;
    uint8_t GetMinute() const;
    uint8_t GetSecond() const;
    size_t ToSeconds() const;
    BTime &operator = (const BTime &src);
};

bool operator == (const BTime &rhs, const BTime &lhs);
bool operator < (const BTime &rhs, const BTime &lhs);
bool operator > (const BTime &rhs, const BTime &lhs);
bool operator <= (const BTime &rhs, const BTime &lhs);
bool operator >= (const BTime &rhs, const BTime &lhs);
#endif

