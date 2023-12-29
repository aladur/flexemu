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
    Underlined = 1, // Print text underline.
    DoubleStrike = 2, // Print twice with a small shift inbetween.
    Emphasized = 4, // Print with half speed gives higher pixel density
    Italic = 8, // Print with italic font.
    DoubleWidth = 16, // Print double width characters.
    SubScript = 32, // Print subscript aligned.
    SuperScript = 64, // Print superscript aligned.
    StrikeThrough = 128, // Print text with strike through.
    PageBreak = 256, // Add page break after this line.
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
    void EvaluateOverlay();
    void EvaluateOverlays();

    std::vector<std::string> overlays;
    std::string currentOverlay;
    RichLine richLine;
    RichLine currentRichLine;
    CharProperty currentProps;
    bool isEscapeSequence;
    std::string escapeSequence;
    int backspaceCount;
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

inline bool operator== (CharProperty lhs, int rhs)
{
    using T1 = std::underlying_type<CharProperty>::type;

    return static_cast<T1>(lhs) == rhs;
}

inline bool operator!= (CharProperty lhs, int rhs)
{
    return !operator==(lhs, rhs);
}
#endif

