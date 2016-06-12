/*
    bdate.h

    Basic class containing a date
    Copyright (C) 1999-2004  W. Schwotzer

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

#ifndef __bdate_h__
#define __bdate_h__

// possible format modes for getDateString:

#define DATE_DEFAULT_FORMAT	(1)
#define DATE_ISO_FORMAT		(2)


class BDate {

private:

	int		day;
	int		month;
	int		year;

public:

	BDate(int d = 0, int m = 0, int year = 0);	// public constructor
	BDate(const BDate& d);				// public constructor
	~BDate();					// public destructor


	static const BDate Now(void);
	static const char *monthName[];
	const char	*GetDateString(int mode = DATE_DEFAULT_FORMAT) const;
	void	Assign(int day, int month, int year);
	void	GetDate(int *day, int *month, int *year);
	void	SetDate(int day, int month, int year);
	void	SetDate(const BDate& date);
	int		GetDay(void) const ;
	int		GetMonth(void) const ;
	int		GetMonthBounded(void) const;
	int		GetYear(void) const;
	BDate& operator = (const BDate& d);
	bool	operator < (const BDate& d) const;
	bool	operator == (const BDate& d) const;
	bool	operator > (const BDate& d) const;
	bool	operator >= (const BDate& d) const;
	bool	operator <= (const BDate& d) const;
private:
	int		MakeComparable(void) const;

	//static int		year2000; // if !0 0 and if year < 70 add 1900 to it
}; // class BDate

inline void BDate::Assign(int d, int m, int y) { day = d; month = m; year = y; }
inline BDate& BDate::operator = (const BDate& d) { Assign(d.day, d.month, d.year); return *this; }
inline int BDate::GetDay(void) const {	return day; }
inline int BDate::GetMonth(void) const { return month; }
inline void BDate::GetDate(int *d, int *m, int *y) {	*d = day; *m = month; *y = year; }
inline void BDate::SetDate(int d, int m, int y) { Assign(d, m, y); }

#endif // #ifndef __bdate_h__
