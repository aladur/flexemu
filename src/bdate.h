/*
    bdate.h

    Basic class containing a date
    Copyright (C) 1999-2024  W. Schwotzer

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

#ifndef BDATE_INCLUDED
#define BDATE_INCLUDED

#include <string>
#include <cstdint>

class BDate
{

private:

    int day{0};
    int month{0};
    int year{0};

public:

    enum class Format : uint8_t
    {
        D2MS3Y4, // DD-MMM-YYYY, MMM are the first three char. of month name
        D2MSU3Y4,// DD-MMM-YYYY, MMM are the first three char. of month name
                 // as uppercase
        Iso, // YYYYMMDD
    };

    BDate() = default;
    BDate(int d, int m, int y);
    BDate(const BDate &src) = default;
    ~BDate() = default;

    static BDate Now();
    std::string GetDateString(Format format = Format::D2MS3Y4) const;
    void Assign(int day, int month, int year);
    void GetDate(int &day, int &month, int &year) const;
    void SetDate(int day, int month, int year);
    void SetDate(const BDate &date);
    int GetDay() const ;
    int GetMonth() const ;
    int GetMonthBounded() const;
    int GetYear() const;
    BDate &operator = (const BDate &src) = default;
    bool operator < (const BDate &d) const;
    bool operator == (const BDate &d) const;
    bool operator > (const BDate &d) const;
    bool operator >= (const BDate &d) const;
    bool operator <= (const BDate &d) const;

private:
    static const char *monthName[];
    int MakeComparable() const;

    //static int year2000; // if !0 0 and if year < 70 add 1900 to it
}; // class BDate

inline void BDate::Assign(int d, int m, int y)
{
    day = d;
    month = m;
    year = y;
}
inline int BDate::GetDay() const
{
    return day;
}
inline int BDate::GetMonth() const
{
    return month;
}
inline void BDate::GetDate(int &d, int &m, int &y) const
{
    d = day;
    m = month;
    y = year;
}
inline void BDate::SetDate(int d, int m, int y)
{
    Assign(d, m, y);
}

#endif // BDATE_INCLUDED
