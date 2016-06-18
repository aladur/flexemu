/*
    bstring.h


    Basic class containing a string definition
    Copyright (C) 1999-2005  W. Schwotzer

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

#ifndef __bstring_h__
#define __bstring_h__

#include "misc1.h"
#ifdef HAVE_STDARG_H
    #include <stdarg.h>  // needed for va_list
#endif

class BString
{

private:
    char    *str;   // pointer to char array
    int     sz;     // allocated size

protected:
    void init(const char *s, int maxlen);
    void initForChar(const char c);

public:
    BString();
    BString(const BString &s, int maxlen = -1);
    BString(const char *s, int maxlen = -1);
    BString(const char c);
    ~BString();

    unsigned int length(void) const;
    bool          empty(void) const;
    const char   *c_str(void) const;
    int          capacity(void) const;

    void cat(const char *s1, const char *s2, BString &s3);
    void cat(BString &s1, BString &s2, BString &s3);
    void cat(const char *s1, BString &s2, BString &s3);
    void cat(BString &s1, const char *s2, BString &s3);

    void alloc(const char *s);
    void alloc(int s);

    BString &operator = (const BString &s);
    BString &operator = (const char *s);
    BString &operator = (const char c);

    BString &operator += (const BString &s);
    BString &operator += (const char *s);
    BString &operator += (const char c);

    BString operator + (const BString &s) const;
    BString operator + (const char *s) const;
    BString operator + (const char c) const;

    char const &operator [] (unsigned int pos) const;

    void at(unsigned int pos, int len, BString &s);

    bool operator < (const BString &s) const;
    bool operator <= (const BString &s) const;
    bool operator == (const BString &s) const;
    bool operator != (const BString &s) const;
    bool operator >= (const BString &s) const;
    bool operator > (const BString &s) const;

    bool operator == (const char *s)    const;
    bool operator != (const char *s)    const;

    void upcase(void);
    void downcase(void);

    void FSadd(const char *s1);

    static const int INVALID_STR;
    static const int STRING_DEFAULT_SIZE;

    friend inline void cat(BString &s1, BString &s2, BString &s3);
    friend inline void cat(const char *s1, BString &s2, BString &s3);
    friend inline void cat(BString &s1, const char *s2, BString &s3);
    friend inline void cat(const char *s1, const char *s2, BString &s3);
};

void FScat(const char *s1, const char *s2, BString &s3);

inline bool BString::empty(void) const
{
    return str[0] == '\0';
}

inline const char *BString::c_str(void) const
{
    return &str[0];
}

inline int BString::capacity(void) const
{
    return sz;
}

inline void cat(BString &s1, BString &s2, BString &s3)
{
    FScat(s1.c_str(), s2.c_str(), s3);
}
inline void cat(const char *s1, BString &s2, BString &s3)
{
    FScat(s1, s2.c_str(), s3);
}
inline void cat(BString &s1, const char *s2, BString &s3)
{
    FScat(s1.c_str(), s2, s3);
}
inline void cat(const char *s1, const char *s2, BString &s3)
{
    FScat(s1, s2, s3);
}

inline BString &BString::operator = (const BString &s)
{
    alloc(s.c_str());
    return *this;
}
inline BString &BString::operator = (const char *_s)
{
    alloc(_s);
    return *this;
}
inline BString &BString::operator = (const char c)
{
    char _s[2];
    _s[0] = c;
    _s[1] = '\0';
    alloc(_s);
    return *this;
}

inline BString &BString::operator += (const BString &s)
{
    FSadd(s.c_str());
    return *this;
}
inline BString &BString::operator += (const char *s)
{
    FSadd(s);
    return *this;
}
inline BString &BString::operator += (const char c)
{
    char s[2];
    s[0] = c;
    s[1] = '\0';
    FSadd(s);
    return *this;
}

inline BString BString::operator + (const BString &s) const
{
    BString r(this->c_str());
    r.FSadd(s.c_str());
    return r;
}
inline BString BString::operator + (const char *s) const
{
    BString r(this->c_str());
    r.FSadd(s);
    return r;
}
inline BString BString::operator + (const char c) const
{
    char s[2];
    s[0] = c;
    s[1] = '\0';
    BString r(this->c_str());
    r.FSadd(s);
    return r;
}

inline char const &BString::operator [] (unsigned int pos) const
{
    return str[pos];
}

#endif // #ifndef __bstring_h__

