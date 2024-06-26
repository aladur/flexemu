/*
    mc146818.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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
#include "mc146818.h"
#include <array>
#include <fstream>
#include <cstring>


const std::array<Byte, 12> days_per_month{
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

Mc146818::Mc146818()
{
    struct tm *lt;
    time_t time_now;
    const auto path = getConfigFilePath();

    if (!path.empty())
    {
        std::ifstream istream(path, std::ios::in | std::ios::binary);

        if (istream.is_open())
        {
            istream.read(reinterpret_cast<char *>(ram.data()),
                         static_cast<int>(ram.size()));
            if (istream.fail())
            {
                std::memset(ram.data(), '\0', ram.size());
            }
        }
    }
    else
    {
        std::memset(ram.data(), '\0', ram.size());
    }

    A = 0;
    B = 0x06;

    // initialize clock registers with system time
    time_now = time(nullptr);
    lt = localtime(&time_now);
    second = convert(static_cast<Byte>(lt->tm_sec));
    minute = convert(static_cast<Byte>(lt->tm_min));
    hour = convert_hour(static_cast<Byte>(lt->tm_hour));
    weekday = static_cast<Byte>(lt->tm_wday + 1);
    day = convert(static_cast<Byte>(lt->tm_mday));
    month = convert(static_cast<Byte>(lt->tm_mon + 1));
    year = convert(static_cast<Byte>(lt->tm_year % 100));
}

std::string Mc146818::getConfigFilePath()
{
    auto path = flx::getHomeDirectory();

    if (!path.empty())
    {
        if (!flx::endsWithPathSeparator(path))
        {
            path.append(PATHSEPARATORSTRING);
        }

        path.append(".mc146818");
    }

    return path;
}

Mc146818::~Mc146818()
{
    const auto path = getConfigFilePath();

    if (!path.empty())
    {
        return;
    }

    std::ofstream ostream(path, std::ios::out | std::ios::binary);

    if (ostream.is_open())
    {
        ostream.write(reinterpret_cast<char *>(ram.data()),
                     static_cast<int>(ram.size()));
    }
}

void Mc146818::resetIo()
{
    A &= 0x7f;
    B &= 0x87;
    C = 0;
    D = 0x80;
}

Byte Mc146818::readIo(Word offset)
{
    Byte temp;

    switch (offset & 0x3f)
    {
        case 0x00:
            return second;

        case 0x01:
            return al_second;

        case 0x02:
            return minute;

        case 0x03:
            return al_minute;

        case 0x04:
            return hour;

        case 0x05:
            return al_hour;

        case 0x06:
            return weekday;

        case 0x07:
            return day;

        case 0x08:
            return month;

        case 0x09:
            return year;

        case 0x0a:
            return A;

        case 0x0b:
            return B;

        case 0x0c:
            temp = C;
            C = 0x00;
            return temp;

        case 0x0d:
            return D;

        default:
            return ram[offset - 0x0e];
    }
}

// writing time not supported

void Mc146818::writeIo(Word offset, Byte val)
{
    switch (offset & 0x3f)
    {
        case 0x00:
            second = val;
            break;

        case 0x01:
            al_second = val;
            break;

        case 0x02:
            minute = val;
            break;

        case 0x03:
            al_minute = val;
            break;

        case 0x04:
            hour = val;
            break;

        case 0x05:
            al_hour = val;
            break;

        case 0x06:
            weekday = val;
            break;

        case 0x07:
            day = val;
            break;

        case 0x08:
            month = val;
            break;

        case 0x09:
            year = val;
            break;

        case 0x0a:
            A = val & 0x7f;
            break;

        case 0x0b:
            B = val;

        // a SET bit going 1 clears the UIE bit
        //if (BSET7(B))
        //   B = val & 0xef;
        case 0x0c:
        case 0x0d:
            break; // Reg. C and D is read only

        default:
            ram[offset - 0x0e] = val;
            break;
    }
}

void Mc146818::update_1_second()
{
    // update only if SET bit is 0
    if (!BTST7(B))
    {
        static Byte dse_october = 0;

        // check for last sunday in april 1:59:59
        if (BTST0(B) && hour == 1 &&
            convert_bin(minute) == 59 &&
            convert_bin(second) == 59 &&
            month == 4 &&
            weekday == 1 &&
            convert_bin(day) >= 24)
        {
            hour = 3;
            minute = 0;
            second = 0;
            // check for last sunday in october 1:59:59
        }
        else if (BTST0(B) && hour == 1 &&
                 convert_bin(minute) == 59 &&
                 convert_bin(second) == 59 &&
                 convert_bin(month) == 10 &&
                 weekday == 1 &&
                 convert_bin(day) >= 25 &&
                 !dse_october)
        {
            dse_october = 1;
            hour = 1;
            minute = 0;
            second = 0;
        }
        else
        {
            // do a normal update
            if (increment(second, 0, 59))
            {
                if (increment(minute, 0, 59))
                {
                    if (increment_hour(hour))
                    {
                        increment(weekday, 1, 7);

                        if (increment_day(day, month, year))
                        {
                            if (increment(month, 1, 12))
                            {
                                increment(year, 0, 99);
                            }
                        }
                    }
                }
            }
        }

        BSET4(C); // set update ended interrupt flag

        if (BTST4(B))
        {
            BSET7(C);
            Notify(NotifyId::SetFirq);
        }

        // now check for an alarm
        if ((((al_second & 0xc0) == 0xc0) || (al_second == second)) &&
            (((al_minute & 0xc0) == 0xc0) || (al_minute == minute)) &&
            (((al_hour & 0xc0) == 0xc0) || (al_hour == hour)))
        {
            BSET5(C); // set alarm interrupt flag

            if (BTST5(B))
            {
                BSET7(C);
                Notify(NotifyId::SetFirq);
            }
        }
    }
}

// convert from binary to binary or bcd
Byte Mc146818::convert(Byte val) const
{
    if (B & 0x04)
    {
        return val;
    }

    return ((val / 10) << 4)  | (val % 10);
}

Byte Mc146818::convert_hour(Byte val) const
{
    switch (B & 0x06)
    {
        case 0x00:      //12 hour, BCD
            if (val >= 12)
            {
                return 0x80 | (((val - 12) / 10) << 4) | ((val - 12) % 10);
            }
            else
            {
                return ((val / 10) << 4)  | (val % 10);
            }

        case 0x02:      //24 hour, BCD
            return ((val / 10) << 4)  | (val % 10);

        case 0x04:      //12 hour, binary
            if (val >= 12)
            {
                return (val - 12) | 0x80;
            }
            else
            {
                return val;
            }

        case 0x06:      //24 hour, binary
            return val;
    }

    return 1; // this should NEVER happen
}

// convert from bcd or binary to binary
Byte Mc146818::convert_bin(Byte val) const
{
    if (B & 0x04)
    {
        return val;
    }

    return ((val >> 4) * 10)  | (val & 0x0f);
}

// return 1 on overflow
bool Mc146818::increment(Byte &reg, Byte min, Byte max)
{
    if (B & 0x04)
    {
        // binary calculation
        reg++;

        if (reg > max)
        {
            reg = min;
            return true;
        }

        return false;
    }

    // bcd calculation
    if ((reg & 0x0f) == 9)
    {
        reg = (reg & 0xf0) + 0x10;
    }
    else
    {
        reg++;
    }

    if (reg > convert(max))
    {
        reg = min;
        return true;
    }

    return false;
}


bool Mc146818::increment_hour(Byte &p_hour) const
{
    switch (B & 0x06)
    {
        case 0x00:      //12 hour, BCD
            if (p_hour == 0x12)
            {
                p_hour = 0x81;
            }
            else if (p_hour == 0x92)
            {
                p_hour = 0x01;
                return true;
            }
            else if ((p_hour & 0x0f) == 9)
            {
                p_hour = (p_hour & 0xf0) + 0x10;
            }
            else
            {
                p_hour++;
            }

            break;

        case 0x02:      //24 hour, BCD
            if ((p_hour & 0x0f) == 9)
            {
                p_hour = (p_hour & 0xf0) + 0x10;
            }
            else if (p_hour == 0x23)
            {
                p_hour = 0x00;
                return true;
            }
            else
            {
                p_hour++;
            }

            break;

        case 0x04:      //12 hour, binary
            if (p_hour == 0x0C)
            {
                p_hour = 0x81;
            }
            else if (p_hour == 0x8C)
            {
                p_hour = 0x01;
                return true;
            }
            else
            {
                p_hour++;
            }

            break;

        case 0x06:      //24 hour, binary
            if (p_hour == 0x17)
            {
                p_hour = 0x00;
                return true;
            }
            else
            {
                p_hour++;
            }

            break;
    }

    return false;
}


bool Mc146818::increment_day(Byte &p_day, Byte p_month, Byte p_year)
{
    Byte binmonth;

    binmonth = convert_bin(p_month);

    if (binmonth < 1)
    {
        binmonth = 1;
    }

    if (binmonth > 12)
    {
        binmonth = 12;
    }

    // if February leap year
    if (binmonth == 2 && (convert_bin(p_year) % 4 == 0))
    {
        if (convert_bin(p_day) == 29)
        {
            // switch to next month on 29. Febr.
            p_day = 1;
            return true;
        }
    }
    else if (convert_bin(p_day) == days_per_month[binmonth - 1])
    {
        p_day = 1;
        return true;
    }

    if (B & 0x04)
    {
        // binary calculation
        p_day++;
    }
    else
    {
        // bcd calculation
        if ((p_day & 0x0f) == 9)
        {
            p_day = (p_day & 0xf0) + 0x10;
        }
        else
        {
            p_day++;
        }
    }

    return false;
}

