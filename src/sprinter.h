/*
    sprinter.h


    Copyright (C) 2015-2022  W. Schwotzer

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

#ifndef SPRINTER_INCLUDED
#define SPRINTER_INCLUDED


#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <iterator>


class sprinter
{
private:
    template<typename T>
    static std::string replaceStringByT(const std::string &format,
                                        const std::string &formatItem,
                                        T& replacement)
    {
       // loop and replace any formatItem by replacement until no
       //formatItem is found any more.
       std::string::iterator it;
       std::string result(format);

       while ((it = std::search(result.begin(), result.end(),
                    formatItem.begin(), formatItem.end())) != result.end())
       {
           std::stringstream stream;
           std::string temp;

           std::copy(result.begin(), it, std::back_inserter(temp));
           stream << temp << replacement;
           temp.clear();
           it += formatItem.size();
           copy(it, result.end(), std::back_inserter(temp));
           stream << temp;
           result = stream.str();
       }

       return result;
    }

    // template for one parameter p1 with index idx
    // The parameter has to support operator<<
    template<typename T>
    static std::string fprint(int idx, const std::string &format, T& p1)
    {
        std::string formatItem = "{" + toString<int>(idx) + "}";

        return replaceStringByT(format, formatItem, p1);
    }

    // template for two parameters p1 and p2 with index idx
    // Each parameter has to support operator<<
    template<typename T1, typename T2>
    static std::string fprint(int idx, const std::string &format, T1 &p1, T2 &p2)
    {
        std::string formatItem = "{" + toString<int>(idx) + "}";
        std::string result = replaceStringByT(format, formatItem, p1);

        return fprint(idx + 1, result, p2);
    }

    // template for three parameters p1, p2 and p3 with index idx
    // Each parameter has to support operator<<
    template<typename T1, typename T2, typename T3>
    static std::string fprint(int idx, const std::string &format, T1 &p1, T2 &p2, T3 &p3)
    {
        std::string formatItem = "{" + toString<int>(idx) + "}";
        std::string result = replaceStringByT(format, formatItem, p1);

        return fprint(idx + 1, result, p2, p3);
    }

    template<typename T>
    static std::string toString(const T &p1)
    {
        std::stringstream result;

        result << p1;

        return result.str();
    }

public:
    // template functions for printing one, two or three parameters of any type into a string
    // using template functions.
    // The syntax of the format string is a subset of the .NET
    // String.Format syntax:
    // {0} is replaced by the first parameter
    // ...
    // {n} is replaced by the nth parameter
    template<typename T1>
    static std::string print(const std::string &format, T1 &p1)
    {
        return fprint(0, format, p1);
    }

    template<typename T1, typename T2>
    static std::string print(const std::string &format, T1 &p1, T2 &p2)
    {
        return fprint(0, format, p1, p2);
    }

    template<typename T1, typename T2, typename T3>
    static std::string print(const std::string &format, T1 &p1, T2 &p2, T3 &p3)
    {
        return fprint(0, format, p1, p2, p3);
    }


    // regex search for ".*\\{[0-9]+\\}.*" in parameter text.
    // using std::regex would be really nice here but produces
    // too much code bloat.
    static inline bool isformatstring(const std::string &text)
    {
        std::string::const_iterator it = text.begin();

        // Parse opening curly brackets
        while ((it = std::find(it, text.end(), '{')) != text.end())
        {
            // Parse one or multiple digits
            std::string::const_iterator it2;
            it2 = std::find_if(++it, text.end(), ::isdigit);

            // If at least one digit parse closing curly brackets
            if ((it != it2) && (it2 != text.end()) && (*it2 == '}'))
            {
                return true;
            }

            // Parse failed. Continue with next non-digit character
            it = it2;
        };

        return false;
    }
};

#endif // SPRINTER_INCLUDED


