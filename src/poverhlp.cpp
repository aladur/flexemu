/*                                                                              
    poverhlp.cpp


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


#include <set>
#include "poverhlp.h"
#include "asciictl.h"

std::string toString(const RichLine &richLine)
{
    std::string result;

    result.reserve(richLine.size());
    for (const auto &richChar : richLine)
    {
        result.push_back(richChar.character);
    }

    return result;
}

PrintOverlayHelper::PrintOverlayHelper()
{
    currentOverlay.reserve(80);
};

bool PrintOverlayHelper::AddCharacter(char character)
{
    static const std::set<char> ignoredCtrlChars { '\0' };

    if (character == CR)
    {
        AddOverlay();
    }
    else if (character == LF)
    {
        EvaluateOverlays();
        return true;
    }
    else if (character >= ' ' && character <= '~')
    {
        currentOverlay.push_back(character);
    } 
    else
    {
        if (ignoredCtrlChars.find(character) == ignoredCtrlChars.end())
        {
            //printf("What to do with character 0x%02X\n", (uint16_t)character);
        }
    }

    return false;
}

const RichLine &PrintOverlayHelper::GetRichLine() const
{
    return richLine;
}

bool PrintOverlayHelper::IsRichLineEmpty() const
{
    return richLine.empty();
}

void PrintOverlayHelper::AddOverlay()
{
    overlays.push_back(currentOverlay);
    currentOverlay.clear();
}

size_t PrintOverlayHelper::GetMaxOverlaySize() const
{
    size_t maxSize = 0U;
    for (const auto &overlay : overlays)
    {
        if (overlay.size() > maxSize)
        {
            maxSize = overlay.size();
        }
    }

    return maxSize;
}


void PrintOverlayHelper::EvaluateOverlays()
{
    richLine.clear();
    auto maxSize = GetMaxOverlaySize();
    while (richLine.size() < maxSize)
    {
        richLine.push_back({ ' ', CharProperty::Normal });
    }

    for (size_t index = 0U; index < maxSize; ++index)
    {
        size_t count = 0U;
        auto character = ' ';
        auto properties = CharProperty::Normal;

        for (size_t oi = 0U; oi < overlays.size(); ++oi)
        {
            const auto &overlay = overlays[oi];

            if (overlay.size() > index)
            {
                if (overlay[index] == '_')
                {
                    properties |= CharProperty::Underlined;
                }
                else if (character == ' ' && overlay[index] != ' ')
                {
                    character = overlay[index];
                    count = 1U;
                }
                else if (character != ' ' && character == overlay[index])
                {
                    ++count;
                }
            }
        }

        if (count == 2U)
        {
            properties |= CharProperty::DoubleStrike;
        }
        else if (count == 3U)
        {
            properties |= CharProperty::Bold;
        }
        else if (count > 3U)
        {
            properties |= CharProperty::DoubleStrike | CharProperty::Bold;
        }

        richLine[index] = { character, properties };
    }
    auto isNoSpace = [](RichCharacter &rc){
        return rc.character != ' ' ||
            (rc.properties & CharProperty::Underlined) != CharProperty::Normal;
    };
    auto result = std::find_if(richLine.begin(), richLine.end(), isNoSpace);
    if (result == richLine.end())
    {
        richLine.clear();
    }

    overlays.clear();
}

void PrintOverlayHelper::Clear()
{
    overlays.clear();
    currentOverlay.clear();
    richLine.clear();
}

