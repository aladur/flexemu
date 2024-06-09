/*
    bdate.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 1998-2024 W. Schwotzer

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
#include "bdate.h"
#include <array>
#include <sstream>
#include <fmt/format.h>

//BDate::year2000 = 1;

BDate::BDate(int d, int m, int y) : day(d), month(m), year(y)
{
}

BDate BDate::Now()
{
    time_t time_now;
    struct tm *lt;
    BDate aTime;

    time_now = time(static_cast<time_t *>(nullptr));
    lt = localtime(&time_now);

    aTime =  BDate(lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900);
    return aTime;
}

const std::array<const char *, 13> BDate::monthNames
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", "???"
};

std::string BDate::GetDateString(Format format) const
{
    std::stringstream stream;
    auto m = GetMonth();
    auto y = GetYear();

    if (m < 1 || m > 12)
    {
        m = 13;
    }

    switch (format)
    {
        case Format::Iso:
            stream << fmt::format("{:04}{:02}{:02}", y, GetMonth(), GetDay());
            break;

        case Format::D2MS3Y4:
            stream << fmt::format("{:02}-{}-{:04}", GetDay(),
                      monthNames[m - 1], y);
            break;

        case Format::D2MSU3Y4:
            std::string monthString(monthNames[m - 1]);
            strupper(monthString);
            stream << fmt::format("{:02}-{}-{:04}", GetDay(), monthString, y);
            break;
    }

    return stream.str();
}

int BDate::GetMonthBounded() const
{
    if (month < 1)
    {
        return 1;
    }

    if (month > 12)
    {
        return 12;
    }

    return month;
}

void BDate::SetDate(const BDate &date)
{
    Assign(date.GetDay(), date.GetMonth(), date.GetYear());
}

int BDate::GetYear() const
{
    if (year < 100)
    {
        if (year < 75)
        {
            return year + 2000;
        }
        return year + 1900;
    }
    if (year < 256)
    {
        return year + 1900; // flexemu: fix bug for year >= 100
    }
    return year;
}

int BDate::MakeComparable() const
{
    return GetYear() * 10000 + (GetMonthBounded() * 100) + GetDay();
}

bool BDate::operator < (const BDate &d) const
{
    return MakeComparable() < d.MakeComparable();
}

bool BDate::operator == (const BDate &d) const
{
    return MakeComparable() == d.MakeComparable();
}

bool BDate::operator != (const BDate &d) const
{
    return MakeComparable() != d.MakeComparable();
}

bool BDate::operator > (const BDate &d) const
{
    return MakeComparable() > d.MakeComparable();
}

bool BDate::operator >= (const BDate &d) const
{
    return !(*this < d);
}

bool BDate::operator <= (const BDate &d) const
{
    return !(*this > d);
}

