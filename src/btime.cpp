/*
    btime.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2022-2023 W. Schwotzer

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
#include <iostream>
#include <iomanip>


BTime::BTime(int h, int m, int s)
    : hour(h)
    , minute(m)
    , second(s)
{
}

BTime::BTime(const BTime &src)
    : hour(src.hour)
    , minute(src.minute)
    , second(src.second)
{
}

BTime::~BTime()
{
}

BTime BTime::Now()
{
    auto time_now = time((time_t *)nullptr);
    auto *lt = localtime(&time_now);

    return BTime(lt->tm_hour, lt->tm_min, lt->tm_sec);
}

const std::string BTime::AsString(Format format) const
{
    std::stringstream stream;

    switch (format)
    {
        case Format::HHMMSS:
            stream << std::setfill('0') <<
                std::setw(2) << hour << ':' <<
                std::setw(2) << minute << ':' <<
                std::setw(2) << second;
            break;

        case Format::HHMM:
            stream << std::setfill('0') <<
                std::setw(2) << hour << ':' <<
                std::setw(2) << minute;
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

BTime &BTime::operator = (const BTime &src)
{
    hour = src.hour;
    minute = src.minute;
    second = src.second;
    return *this;
}

bool operator == (const BTime &rhs, const BTime &lhs)
{
    return rhs.ToSeconds() == lhs.ToSeconds();
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

