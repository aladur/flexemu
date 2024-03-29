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


#include <stdio.h>

#include "misc1.h"
#include "mc146818.h"
#include "mc6809.h"
#include "bfileptr.h"

Byte last_day[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

Mc146818::Mc146818() :
    al_second(0), al_minute(0), al_hour(0),
    A(0), B(0), C(0), D(0),
    path("")
{
    struct tm   *lt;
    time_t time_now;
    const char* home = getFileName();

    if (home[0] != '\0')
    {
        BFilePtr fp(home, "rb");

        if (fp != nullptr)
        {
            if (fread(ram, 1, sizeof(ram), fp) != sizeof(ram))
            {
                memset(ram, 0, sizeof(ram));
            }
        }
    }
    else
    {
        memset(ram, 0, sizeof(ram));
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

const char *Mc146818::getFileName()
{
    if (path[0] == '\0')
    {
#ifdef UNIX
        char *home = getenv("HOME");
#endif
#ifdef _WIN32
        char* home = getenv("USERPROFILE");
#endif

        if (home != nullptr)
        {
            strcpy(path, home);
            strcat(path, PATHSEPARATORSTRING);
        }

        if (path[0] != '\0')
        {
            strcat(path, ".mc146818");
        }
    }

    return path;
}

Mc146818::~Mc146818()
{
    const char* home = getFileName();

    if (home[0] != '\0')
    {
        BFilePtr fp(home, "wb");

        if (fp != nullptr)
        {
            fwrite(ram, 1, sizeof(ram), fp);
        }
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
} // update

// convert from binary to binary or bcd
Byte Mc146818::convert(Byte val)
{
    if (B & 0x04)
    {
        return val;
    }

    return ((val / 10) << 4)  | (val % 10);
}

Byte Mc146818::convert_hour(Byte val)
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
    }  // switch

    return 1; // this should NEVER happen
}

// convert from bcd or binary to binary
Byte Mc146818::convert_bin(Byte val)
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


bool Mc146818::increment_hour(Byte &x_hour)
{
    switch (B & 0x06)
    {
        case 0x00:      //12 hour, BCD
            if (x_hour == 0x12)
            {
                x_hour = 0x81;
            }
            else if (x_hour == 0x92)
            {
                x_hour = 0x01;
                return true;
            }
            else if ((x_hour & 0x0f) == 9)
            {
                x_hour = (x_hour & 0xf0) + 0x10;
            }
            else
            {
                x_hour++;
            }

            break;

        case 0x02:      //24 hour, BCD
            if ((x_hour & 0x0f) == 9)
            {
                x_hour = (x_hour & 0xf0) + 0x10;
            }
            else if (x_hour == 0x23)
            {
                x_hour = 0x00;
                return true;
            }
            else
            {
                x_hour++;
            }

            break;

        case 0x04:      //12 hour, binary
            if (x_hour == 0x0C)
            {
                x_hour = 0x81;
            }
            else if (x_hour == 0x8C)
            {
                x_hour = 0x01;
                return true;
            }
            else
            {
                x_hour++;
            }

            break;

        case 0x06:      //24 hour, binary
            if (x_hour == 0x17)
            {
                x_hour = 0x00;
                return true;
            }
            else
            {
                x_hour++;
            }

            break;
    }  // switch

    return false;
}


bool Mc146818::increment_day(Byte &x_day, Byte x_month, Byte x_year)
{
    Byte binmonth;

    binmonth = convert_bin(x_month);

    if (binmonth < 1)
    {
        binmonth = 1;
    }

    if (binmonth > 12)
    {
        binmonth = 12;
    }

    // if February leap year
    if (binmonth == 2 && (convert_bin(x_year) % 4 == 0))
    {
        if (convert_bin(x_day) == 29)
        {
            // switch to next month on 29. Febr.
            x_day = 1;
            return true;
        }
    }
    else if (convert_bin(x_day) == last_day[binmonth - 1])
    {
        x_day = 1;
        return true;
    }

    if (B & 0x04)
    {
        // binary calculation
        x_day++;
    }
    else
    {
        // bcd calculation
        if ((x_day & 0x0f) == 9)
        {
            x_day = (x_day & 0xf0) + 0x10;
        }
        else
        {
            x_day++;
        }
    }

    return false;
}

