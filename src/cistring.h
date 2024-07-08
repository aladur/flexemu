/*
   Copyright

   This code is copyright (C) 1996-2005 by Herb Sutter unless otherwise noted.
   All rights reserved.

   For details see Herb Sutters copyright: http://www.gotw.ca/copyright.htm
*/


#include <string>

/*
 Define a case insensitive char trait.
 Together with a ci_string type definition it allows a
 case insensitive string comparison with the
 ci_string.compare() member function or equality operator (operator==).
 This work is based on: Guru of the Week #29, http://www.gotw.ca/gotw/029.htm

 Example:
 #include "cistring.h"
 ci_string ci_str("AbCdE");
 ci_str.compare("ABCDE"); // returns 0
 ci_str.compare("abcde"); // returns 0
 ci_str == "ABCDE"; // is true
 ci_str == "abcde"; // is true
*/

struct ci_char_traits : public std::char_traits<char>
{
    static bool eq(char c1, char c2)
    {
        return ::tolower(c1) == ::tolower(c2);
    }

    static bool ne(char c1, char c2)
    {
        return ::tolower(c1) != ::tolower(c2);
    }

    static bool lt(char c1, char c2)
    {
        return ::tolower(c1) < ::tolower(c2);
    }

    static int compare(const char* c1, const char* c2, size_t n)
    {
        while (n-- != 0)
        {
            if (::tolower(*c1) < ::tolower(*c2))
            {
                return -1;
            }
            if (::tolower(*c1) > ::tolower(*c2))
            {
                return 1;
            }
            ++c1;
            ++c2;
        }
        return 0;
    }

    static const char* find(const char* s, int n, char a)
    {
        const auto lc_a = ::tolower(a);

        while (n-- > 0 && ::tolower(*s) != lc_a)
        {
            ++s;
        }
        return s;
    }
};

using ci_string = std::basic_string<char, ci_char_traits>;

