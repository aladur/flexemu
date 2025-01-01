/*
    btime.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
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

#include "btime.h"
#include <sstream>
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"


BTime::BTime(int h, int m, int s)
    : hour(h)
    , minute(m)
    , second(s)
{
}

BTime BTime::Now()
{
    const auto time_now = time(nullptr);
    const auto *lt = localtime(&time_now);

    return {lt->tm_hour, lt->tm_min, lt->tm_sec};
}

std::string BTime::AsString(Format format) const
{
    std::stringstream stream;

    switch (format)
    {
        case Format::HHMMSS:
            stream << fmt::format("{:02}:{:02}:{:02}", hour, minute, second);
            break;

        case Format::HHMM:
            stream << fmt::format("{:02}:{:02}", hour, minute);
            break;
    }

    return stream.str();
}

void BTime::Get(int &h, int &m, int &s) const
{
    h = hour;
    m = minute;
    s = second;
}

void BTime::Set(int h, int m, int s)
{
    hour = h;
    minute = m;
    second = s;
}

void BTime::Set(const BTime &src)
{
    hour = src.hour;
    minute = src.minute;
    second = src.second;
}

int BTime::GetHour() const
{
    return hour;
}

int BTime::GetMinute() const
{
    return minute;
}

int BTime::GetSecond() const
{
    return second;
}

size_t BTime::ToSeconds() const
{
    return static_cast<size_t>(hour) * 3600U + minute * 60U + second;
}

bool operator == (const BTime &rhs, const BTime &lhs)
{
    return rhs.ToSeconds() == lhs.ToSeconds();
}

bool operator != (const BTime &rhs, const BTime &lhs)
{
    return rhs.ToSeconds() != lhs.ToSeconds();
}

bool operator < (const BTime &rhs, const BTime &lhs)
{
    return rhs.ToSeconds() < lhs.ToSeconds();
}

bool operator > (const BTime &rhs, const BTime &lhs)
{
    return rhs.ToSeconds() > lhs.ToSeconds();
}

bool operator <= (const BTime &rhs, const BTime &lhs)
{
    return !(rhs > lhs);
}

bool operator >= (const BTime &rhs, const BTime &lhs)
{
    return !(rhs < lhs);
}

