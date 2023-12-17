/*                                                                              
    poverhlp.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2023  W. Schwotzer

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

#ifndef PRINTOVERLAYHELPER_INCLUDED
#define PRINTOVERLAYHELPER_INCLUDED

#include <string>
#include <vector>
#include <algorithm>


enum class CharProperty
{
    Normal = 0,
    Underlined = 1,
    DoubleStrike = 2,
    Bold = 4,
};

struct RichCharacter
{
    char character;
    CharProperty properties;
};

using RichLine = std::vector<RichCharacter>;
using RichLines = std::vector<RichLine>;

std::string toString(const RichLine &richLine);

class PrintOverlayHelper
{
public:
    PrintOverlayHelper();

    bool AddCharacter(char character);
    const RichLine &GetRichLine() const;
    bool IsRichLineEmpty() const;
    void Clear();

private:
    size_t GetMaxOverlaySize() const;
    void AddOverlay();
    void EvaluateOverlays();

    std::vector<std::string> overlays;
    std::string currentOverlay;
    RichLine richLine;

};

inline CharProperty operator| (CharProperty lhs, CharProperty rhs)
{
    using T1 = std::underlying_type<CharProperty>::type;

    return static_cast<CharProperty>(static_cast<T1>(lhs) |
                                     static_cast<T1>(rhs));
}

inline CharProperty operator& (CharProperty lhs, CharProperty rhs)
{
    using T1 = std::underlying_type<CharProperty>::type;

    return static_cast<CharProperty>(static_cast<T1>(lhs) &
                                     static_cast<T1>(rhs));
}

inline CharProperty &operator|= (CharProperty &lhs, CharProperty rhs)
{
    return lhs = lhs | rhs;
}

inline CharProperty &operator&= (CharProperty &lhs, CharProperty rhs)
{
    return lhs = lhs & rhs;
}

inline CharProperty operator~ (CharProperty rhs)
{
    using T1 = std::underlying_type<CharProperty>::type;

    return static_cast<CharProperty>(~static_cast<T1>(rhs));
}

inline bool operator== (CharProperty lhs, CharProperty rhs)
{
    using T1 = std::underlying_type<CharProperty>::type;

    return static_cast<T1>(lhs) == static_cast<T1>(rhs);
}

inline bool operator!= (CharProperty lhs, CharProperty rhs)
{
    return !operator==(lhs, rhs);
}

#endif
