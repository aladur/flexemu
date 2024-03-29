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

class BDate
{

private:

    int day;
    int month;
    int year;

public:

    enum class Format
    {
        D2MS3Y4, // DD-MMM-YYYY, MMM are the first three char. of month name
        D2MSU3Y4,// DD-MMM-YYYY, MMM are the first three char. of month name
                 // as uppercase
        Iso, // YYYYMMDD
    };

    BDate(int d = 0, int m = 0, int year = 0); // public constructor
    BDate(const BDate &d); // public constructor
    ~BDate(); // public destructor


    static const BDate Now();
    const std::string GetDateString(Format format = Format::D2MS3Y4) const;
    void Assign(int day, int month, int year);
    void GetDate(int *day, int *month, int *year);
    void SetDate(int day, int month, int year);
    void SetDate(const BDate &date);
    int GetDay() const ;
    int GetMonth() const ;
    int GetMonthBounded() const;
    int GetYear() const;
    BDate &operator = (const BDate &d);
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
inline BDate &BDate::operator = (const BDate &d)
{
    Assign(d.day, d.month, d.year);
    return *this;
}
inline int BDate::GetDay() const
{
    return day;
}
inline int BDate::GetMonth() const
{
    return month;
}
inline void BDate::GetDate(int *d, int *m, int *y)
{
    *d = day;
    *m = month;
    *y = year;
}
inline void BDate::SetDate(int d, int m, int y)
{
    Assign(d, m, y);
}

#endif // BDATE_INCLUDED
