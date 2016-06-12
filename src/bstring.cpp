/*
    bstring.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2004  W. Schwotzer

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

#include <misc1.h>
#include <ctype.h>
#include <stdio.h>
#include <limits.h>

#include "bstring.h"

const int BString::INVALID_STR         = -1;
const int BString::STRING_DEFAULT_SIZE = 16;

#ifdef __GNUC__
//extern int vsnprintf(char *buf, size_t size, const char *format, va_list args);
#endif

void BString::initForChar(const char c)
{
	sz = 2;
	str = new char[sz];
	str[0] = c;
	str[1] = '\0';
}

void BString::init(const char *s, int maxlen)
{
	if (maxlen == 0 || s == NULL) {
		initForChar('\0');
		return;
	}
	if (maxlen < 0)
		sz = strlen(s) + 1;
	else
		sz = maxlen + 1;
	if (sz < 2)
		sz = 2;

  char *pTmp = str;
	str = new char[sz];
	strncpy(str, s, sz-1);
	str[sz-1] = '\0';
  delete [] pTmp;
}

BString::BString(void) : str(NULL), sz(0)
{
	initForChar('\0');
}

BString::BString(const char c) : str(NULL), sz(0)
{
	initForChar(c);
}

BString::BString(const char *s, int maxlen) : str(NULL), sz(0)
{
  if (s == NULL)
    initForChar('\0');
  else
    init(s, maxlen);
}

BString::BString(const BString& s, int maxlen) : str(NULL), sz(0)
{
	init(s.chars(), maxlen);
}

BString::~BString(void)
{
	delete [] str;
	sz = 0;
}

void FScat(const char *s1, const char *s2, BString& s3)
{
  if (s1 == NULL || s2 == NULL) return;

  int size;

	size = strlen(s1) + strlen(s2) + 1;
	char *newS = new char[size];
	strcpy(newS, s1);
	strcat(newS, s2);
	s3 = BString(newS);
	delete [] newS;
}

void BString::FSadd(const char *s1)
{
  if (s1 == NULL) return;

  int size = length() + strlen(s1) + 1;
	if (size <= allocation())
		strcat(str, s1);
	else {
		char *newS = new char[size];
		strcpy(newS, str);
		strcat(newS, s1);
		delete [] str;
		str = newS;
		sz = size;
	}
}

int BString::index(const char *s, int startpos) const
{
  if (s == NULL) return -1;

  int i1, i2, l1, l2;
	const char *p1, *p2;

	p1 = chars();
	p2 = s;
	l1 = strlen(p1);
	l2 = strlen(p2);
	for (i1 = startpos; i1 < l1; i1++) {
		for (i2 = 0; i2 < l2; i2++) {
			if (*(p1+i1+i2) != *(p2+i2))
				break;
		};
		if (i2 >= l2)
			return i1;
	}
	return -1;
}

void BString::alloc(int s)
{
	if (s > sz) {
		sz = s;
		char *newS = new char[s];
		strcpy(newS, str);
		delete [] str;
		str = newS;
	}
}
void BString::alloc(const char *s)
{
  if (s == NULL) return;

  int size;

	size = strlen(s) + 1;
	if (size > sz) {
		sz = size;
		char *newS = new char[size];
		strcpy(newS, s);
		delete [] str;
		str = newS;
	} else
		strcpy(str, s);
}

void BString::reverse(void)
{
	int i;
	char *p1, c;

	i = 1;
	if (this->length() % 2 == 1) {
		// odd number of characters
		p1 = &str[0] + this->length() / 2;
		while(*(p1+i) != '\0') {
			c = *(p1+i); *(p1+i) = *(p1-i); *(p1-i) = c; i++;
		};
	} else {
		// even number of characters
		p1 = &str[0] + this->length() / 2 - 1;
		while(*(p1+i) != '\0') {
			c = *(p1+i); *(p1+i) = *(p1-i+1); *(p1-i+1) = c; i++;
		};
	}

} 

void BString::upcase(void)
{
	char *p;

	p = &str[0];
	while (*p != '\0')
	{
		*p = toupper(*p);
		p++;
	}
} 

void BString::downcase(void)
{
	char *p;
	
	p = &str[0];
	while (*p != '\0')
	{
		*p = tolower(*p);
		p++;
	}
} 

bool BString::matches(const char *pattern, bool ignorecase /* = false */ ) const
{
	const char *p_pat = pattern;
	const char *p_src = str;
	char char_pat     = '*'; // prepare for first while loop
	char char_src;
	int min = 0;
	int max = 0;
	int notmatched =  0;

	if (pattern == NULL)
		return false;

	while (*p_src != '\0')
	{
		char_src = *p_src;
		if (ignorecase)
			char_src = tolower(char_src);

		while (notmatched == 0 && char_pat != '\0')
		{
			char_pat = *p_pat;
			p_pat++;
			if (char_pat == '*')
			{
				// wildchard for any char
				max = INT_MAX;
				continue;
			} else
			if (char_pat == '?')
			{
				// wildchard for exactly one char
				min++;
				if (max < INT_MAX)
					max++;
				continue;
			} else
			if (char_pat != '\0')
			{
				// any other character
				if (ignorecase)
					char_pat = tolower(char_pat);
				break;
			}
		}
		if (char_src == char_pat)
		{
			if (notmatched < min || notmatched > max)
				return false;
			notmatched = 0;
			min = max = 0;
		} else {
			notmatched++;
			if (notmatched > max)
				return false;
		}
		p_src++;
	}
	if (notmatched < min || notmatched > max)
		return false;
	return (char_pat == '\0' && notmatched > 0) || //pattern ends with ? or *
		(*p_pat == '\0' && notmatched == 0); // pattern end with any char
}

bool BString::multimatches(const char *multipattern,
			const char delimiter /* = ';'*/,
			bool ignorecase /* = false */) const

{
	int begin, pos;
	BString *pattern;

	if (multipattern == NULL)
		return false;

	pos = 0;
	while (multipattern[pos] != '\0')
	{
		begin = pos;
		while (multipattern[pos] != '\0' && (multipattern[pos] != delimiter))
			pos++;
		pattern = new BString(&multipattern[begin], pos - begin);
		if (matches(*pattern, ignorecase))
		{
			delete pattern;
			return true;
		}
		delete pattern;
		if (multipattern[pos] == delimiter)
			pos++;
	}
	return false;
}

char BString::firstchar(void) const
{
	return str[0];
}

char BString::lastchar(void) const
{
	return str[strlen(str)-1];
}

void BString::at(unsigned int pos, int len, BString& s)
{
	if (pos >= strlen(str))
		s = "";
	else
		s = BString(&str[pos], len);
}

int BString::printf(const char *format, ...)
{
  if (format == NULL) return 0;

  int      size, res;
	char     *newS;
	va_list  arg_ptr;

	size = 80;
	do {
		size *= 2;
		if (size > sz) {
			newS = new char[size];
			delete [] str;
			str = newS;
			sz = size;
		}
		va_start(arg_ptr, format);
#ifdef _MSC_VER
		res = _vsnprintf(str, sz, format, arg_ptr);
#endif
#ifdef __GNUC__
		res = vsnprintf(str, sz, format, arg_ptr);
#endif
		va_end(arg_ptr);
		if (res >= 0 && res > sz)
			res = -1;
	} while (res < 0);
	return length();
}

int BString::vsprintf(const char *format, va_list arg_ptr)
{
  if (format == NULL) return 0;

  int     size, res;
	char    *newS;

	size = 80;
	do {
		size *= 2;
		if (size > sz) {
			newS = new char[size];
			delete [] str;
			str = newS;
			sz = size;
		}
#ifdef _MSC_VER
		res = _vsnprintf(str, sz, format, arg_ptr);
#endif
#ifdef __GNUC__
		res = vsnprintf(str, sz, format, arg_ptr);
#endif
		if (res >= 0 && res > sz)
			res = -1;
	} while (res < 0);
	return length();
}

BString& BString::operator += (const int i)
{
	char s[16];

	sprintf((char *)s, "%d", i);
	FSadd((char *)s);
	return *this;
}

BString& BString::operator += (const unsigned int ui)
{
	char s[16];

	sprintf((char *)s, "%ud", ui);
	FSadd((char *)s);
	return *this;
}

void BString::replaceall(const char oldC, const char newC)
{
	if (oldC == '\0' || newC == '\0')
		return;
	for (int i = 0; i < sz && str[i] != '\0'; i++)
		if (str[i] == oldC)
			str[i] = newC;
}

BString BString::beforeLast(const char c) const
{
	BString result;
	int i;

	result = *this;
	for (i = strlen(str)-1; i >= 0; i--)
		if (str[i] == c)
			break;
	if (i >= 0)
		result.str[i] = '\0';
	return result;
}

BString BString::afterLast(const char c) const
{
	BString result;
	int i;

	result = *this;
	for (i = strlen(str)-1; i >= 0; i--)
		if (str[i] == c)
			break;
	if (i >= 0)
		result = (char *)&str[i];
	return result;
}

int BString::comparenocase(const BString &s)
{
	return stricmp(str, s.str);
}

bool BString::operator <  (const BString &s) const
{
	return strcmp(str, s.str)  < 0;
}

bool BString::operator <= (const BString &s) const
{
	return strcmp(str, s.str)  <= 0;
}

bool BString::operator == (const BString &s) const
{
	return strcmp(str, s.str)  == 0;
}

bool BString::operator == (const char *s) const
{
  if (s == NULL) return false;

  return strcmp(str, s)  == 0;
}

bool BString::operator != (const BString &s) const
{
	return strcmp(str, s.str)  != 0;
}

bool BString::operator != (const char *s) const
{
  if (s == NULL) return false;

  return strcmp(str, s)  != 0;
}

bool BString::operator >= (const BString &s) const
{
	return strcmp(str, s.str)  >= 0;
}

bool BString::operator >  (const BString &s) const
{
	return strcmp(str, s.str)  > 0;
}

unsigned int BString::length(void) const
{
  return strlen(str);
}


