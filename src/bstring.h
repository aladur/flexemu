/*
    bstring.h


    Basic class containing a string
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

#ifndef __bstring_h__
#define __bstring_h__

#include <stdarg.h>  // needed for va_list


class BString {

private:
	char    *str;   // pointer to char array
	int     sz;     // allocated size

protected:
	void init(const char *s, int maxlen);
	void initForChar(const char c);

public:
	BString();
	BString(const BString& s, int maxlen = -1);
	BString(const char *s, int maxlen = -1);
	BString(const char c);
	virtual ~BString();

	unsigned int length(void) const;
	bool          empty(void) const;
	const char   *chars(void) const;
	int          allocation(void) const;

	void cat(const char *s1, const char *s2, BString& s3);
	void cat(BString& s1, BString& s2, BString& s3);
	void cat(const char *s1, BString& s2, BString& s3);
	void cat(BString& s1, const char *s2, BString& s3);

	void alloc(const char *s);
	void alloc(int s);

	BString& operator = (const BString& s);
	BString& operator = (const char *s);
	BString& operator = (const char c);

	BString& operator += (const int i);
	BString& operator += (const unsigned int ui);
	BString& operator += (const BString& s);
	BString& operator += (const char *s);
	BString& operator += (const char c);

	BString operator + (const BString& s) const;
	BString operator + (const char *s) const;
	BString operator + (const char c) const;

	bool contains(const BString& s) const;
	bool contains(const char *s) const;
	bool contains(const char c) const;

	int index(const BString& s, int startpos = 0) const;
	int index(const char *s, int startpos = 0) const;
	int index(const char c, int startpos = 0) const;

	BString beforeLast(const char c) const;
	BString afterLast(const char c) const;

	void at(unsigned int pos, int len, BString& s);

	int comparenocase(const BString &s);
	void replaceall(const char oldC, const char newC);

	bool operator <  (const BString &s) const;
	bool operator <= (const BString &s) const;
	bool operator == (const BString &s) const;
	bool operator != (const BString &s) const;
	bool operator >= (const BString &s) const;
	bool operator >  (const BString &s) const;

	bool operator == (const char *s)    const;
	bool operator != (const char *s)    const;

	void reverse(void);
	void upcase(void);
	void downcase(void);

	bool matches(const char *pattern, bool ignorecase = false) const;
	bool multimatches(const char *multipattern,
                        const char delimiter = ';',
                        bool ignorecase = false) const;
	
	char firstchar(void) const;
	char lastchar(void) const;

	// implicit type conversions !
	operator const char *() const ;

	int printf(const char *format, ...);
	int vsprintf(const char *format, va_list arg_ptr);

	void FSadd(const char *s1);

  static const int INVALID_STR;
  static const int STRING_DEFAULT_SIZE;

	friend inline void cat(BString& s1, BString& s2, BString& s3);
	friend inline void cat(const char *s1, BString& s2, BString& s3);
	friend inline void cat(BString& s1, const char *s2, BString& s3);
	friend inline void cat(const char *s1, const char *s2, BString& s3);
};

void FScat(const char *s1, const char *s2, BString& s3);

inline bool BString::empty(void) const { return str[0] == '\0'; };
inline const char *BString::chars(void) const { return &str[0]; };
inline int BString::allocation(void) const { return sz; };

inline void cat(BString& s1, BString& s2, BString& s3) { FScat(s1.chars(), s2.chars(), s3); };
inline void cat(const char *s1, BString& s2, BString& s3) { FScat(s1, s2.chars(), s3); };
inline void cat(BString& s1, const char *s2, BString& s3) { FScat(s1.chars(), s2, s3); };
inline void cat(const char *s1, const char *s2, BString& s3) { FScat(s1, s2, s3); };

inline BString& BString::operator = (const BString& s) { alloc(s.chars()); return *this; };
inline BString& BString::operator = (const char *_s) { alloc(_s); return *this; };
inline BString& BString::operator = (const char c) { char _s[2]; _s[0] = c; _s[1] = '\0'; alloc(_s); return *this; };

inline BString& BString::operator += (const BString& s) { FSadd(s.chars()); return *this; };
inline BString& BString::operator += (const char *s) { FSadd(s); return *this; };
inline BString& BString::operator += (const char c) { char s[2]; s[0] = c; s[1] = '\0'; FSadd(s); return *this; };

inline BString BString::operator + (const BString& s) const  { BString r(this->chars()); r.FSadd(s.chars()); return r; };
inline BString BString::operator + (const char *s) const { BString r(this->chars()); r.FSadd(s); return r; };
inline BString BString::operator + (const char c) const { char s[2]; s[0] = c; s[1] = '\0'; BString r(this->chars()); r.FSadd(s); return r; };

inline int BString::index (const BString& _s, int startpos) const { return index(_s.chars(), startpos); };
inline int BString::index (const char c, int startpos) const { char s[2]; s[0] = c; s[1] = '\0';  return this->index(s, startpos); };

inline bool BString::contains (const BString& _s) const { return index(_s.chars()) >= 0; };
inline bool BString::contains (const char *_s) const { return index(_s) >= 0; };
inline bool BString::contains (const char c) const { char _s[2]; _s[0] = c; _s[1] = '\0'; return index(_s) >= 0; };

inline BString::operator const char *() const { return chars(); };

#endif // #ifndef __bstring_h__

