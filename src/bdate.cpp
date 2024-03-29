/*
    bdate.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
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

#include "bdate.h"
#include "misc1.h"
#include <stdio.h>

//BDate::year2000 = 1;

BDate::BDate(int d, int m, int y) : day(d), month(m), year(y)
{
}

BDate::BDate(const BDate &d)  : day(d.GetDay()), month(d.GetMonth()),
    year(d.GetYear())
{
}

const BDate BDate::Now()
{
    time_t      time_now;
    struct tm   *lt;
    BDate   aTime;

    time_now    = time((time_t *)nullptr);
    lt      = localtime(&time_now);

    aTime =  BDate(lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900);
    return aTime;
}

BDate::~BDate()
{
}

const char *BDate::monthName[] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", "???"
};

const std::string BDate::GetDateString(Format format) const
{
    char dateString[32];
    auto m = GetMonth();
    auto y = GetYear();

    if (m < 1 || m > 12)
    {
        m = 13;
    }

    switch (format)
    {
        case Format::Iso:
            snprintf(dateString, sizeof(dateString), "%04d%02d%02d", y,
                     GetMonth(), GetDay());
            break;

        case Format::D2MS3Y4:
            snprintf(dateString, sizeof(dateString), "%02d-%s-%04d", GetDay(),
                     monthName[m - 1], y);
            break;

        case Format::D2MSU3Y4:
            std::string monthString(monthName[m - 1]);
            strupper(monthString);
            snprintf(dateString, sizeof(dateString), "%02d-%s-%04d", GetDay(),
                     monthString.c_str(), y);
            break;
    } // switch

    return dateString;
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
        return year + 1900;    // flexemu: fix bug for year >= 100
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

