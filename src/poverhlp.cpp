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
#include <functional>
#include "poverhlp.h"
#include "asciictl.h"
#include "bscopeex.h"

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

RichLine CreateRichLine(size_t count, const RichCharacter &rc)
{
    RichLine richLine;

    richLine.reserve(count);
    for (auto i = 0U; i < count; ++i)
    {
        richLine.push_back(rc);
    }

    return richLine;
}

void RichLineAppend(RichLine &richLine, size_t count, const RichCharacter &rc)
{
    for (auto i = 0U; i < count; ++i)
    {
        richLine.push_back(rc);
    }
}

PrintOverlayHelper::PrintOverlayHelper() :
      currentProps(CharProperty::Normal)
    , isEscapeSequence(false)
    , backspaceCount(0)

{
    currentOverlay.reserve(80);
};

bool PrintOverlayHelper::AddCharacter(char character)
{
    // Escape sequences using two characters.
    static const std::set<char> twoCharEscSeq {
        '-', // Underline mode.
        'S', // Super/subscript mode.
        'A', // Specify amount of line spacing in multiples of 1/72 inch, uns.
        'C', // Specify form length per page, unsupported.
    };
    // Escape sequences using multiple characters (> 2 characters)
    static const std::set<char> multiCharEscSeq {
        'B', // Specify vertical TAB positions, unsupported
        'D', // Specify horizontal TAB positions, unsupported
    };
    static const std::set<char> ignoredCtrlChars {
        NUL, // Used for padding or end of ESC B, ESC D.
        BEL, // Make a buzzer sound, unsupported.
        HT, // Position print head to next hor. TAB position, unsupported.
        VT, // Position paper to next vert. TAB position, unsupported.
        SI, // Print in condensed mode, unsupported.
        DC1, // Set printer in selected state, unsupported.
        DC2, // Cancel SI mode, unsupported.
        DC3, // set printer in deselected state, unsupported.
        CAN, // Clear print buffer, unsupported.
        DEL, // Clear print buffer, unsupported.
    };

    currentProps &= ~CharProperty::PageBreak;

    if (isEscapeSequence)
    {
        if (escapeSequence.size() == 0)
        {
            if ((twoCharEscSeq.find(character) != twoCharEscSeq.end()) ||
                (multiCharEscSeq.find(character) != multiCharEscSeq.end()))
            {
                escapeSequence.push_back(character);
                return false;
            }
            else
            {
                switch (character)
                {
                    case SO: // Double width mode on.
                        currentProps |= CharProperty::DoubleWidth;
                        break;
                    case DC4: // Double width mode off.
                        currentProps &= ~CharProperty::DoubleWidth;
                        break;
                    case '4': // Italic mode on.
                        currentProps |= CharProperty::Italic;
                        break;
                    case '5': // Italic mode off.
                        currentProps &= ~CharProperty::Italic;
                        break;
                    case 'E': // Emphasized mode on.
                        currentProps |= CharProperty::Emphasized;
                        break;
                    case 'F': // Emphasized mode off.
                        currentProps &= ~CharProperty::Emphasized;
                        break;
                    case 'G': // Double-strike mode on.
                        currentProps |= CharProperty::DoubleStrike;
                        break;
                    case 'H': // Double-strike mode off.
                        currentProps &= ~CharProperty::DoubleStrike;
                        break;
                    case 'T': // Super/subscript mode off.
                        currentProps &= ~(CharProperty::SubScript |
                                          CharProperty::SuperScript);
                        break;
                    // ESC SI Print in condensed mode, unsupported.
                    // ESC 0  Set line spacing to 1/8 inch, unsupported.
                    // ESC 1  Set line spacing to 7/72 inch, unsupported.
                    // ESC 2  Set line spacing to 1/6 inch, unsupported.
                    // ESC 8  Receive data even if there is no paper in, unsup.
                    // ESC 9  Noreceive data if there is no paper in, unsup.
                }
            }
        }
        else if (escapeSequence.size() == 1)
        {
            switch (escapeSequence[0])
            {
                case '-':
                    if (character == '1') // Begin underline mode
                    {
                        currentProps |= CharProperty::Underlined;
                    }
                    else if (character == '0') // End underline mode
                    {
                        currentProps &= ~CharProperty::Underlined;
                    }
                    break;

                case 'S': // Begin super/subscript mode, unsupported
                    if (character == '1') // Begin subscript mode
                   {
                        currentProps |= CharProperty::SubScript;
                    }
                    else if (character == '0') // Begin superscript mode
                    {
                        currentProps |= CharProperty::SuperScript;
                    }
                    break;
            }
        }
        else // ESC sequence with more than two characters
        {
            // ESC B or ESC D
            if (character != NUL) // NUL is end of escape sequence.
            {
                escapeSequence.push_back(character);
                return false;
            }
        }
        isEscapeSequence = false;
    }
    else if (character == CR) // Carriage return.
    {
        backspaceCount = 0;
        AddOverlay();
    }
    else if (character == LF) // Line feed.
    {
        auto index = currentOverlay.size() - backspaceCount;

        backspaceCount = 0;
        EvaluateOverlays();

        if (index > 0)
        {
            RichCharacter richChar{' ', CharProperty::Normal};
            currentOverlay = std::string(index, ' ');
            currentRichLine = CreateRichLine(index, richChar);
        }
        return true;
    }
    else if (character == ESC)
    {
        isEscapeSequence = true;
        escapeSequence.clear();
    }
    else if (character >= ' ' && character <= '~')
    {
        BScopeExit<std::function<void()> > executeBeforeScopeExit([&](){
            backspaceCount -= (backspaceCount != 0) ? 1 : 0;
        });

        if (backspaceCount == 0)
        {
            currentOverlay.push_back(character);
        }
        else
        {
            const auto index = currentOverlay.size() - backspaceCount;

            if (currentOverlay[index] == ' ')
            {
                currentOverlay[index] = character;
            }
        }

        // The following rich line commands are not needed when using overlays.
        if (!overlays.empty())
        {
            return false;
        }

        if (backspaceCount == 0 || currentRichLine.empty())
        {
            currentRichLine.push_back( { character, currentProps } );
            return false;
        }

        const auto index = currentRichLine.size() > backspaceCount ?
            currentRichLine.size() - backspaceCount : 0;

        switch (character)
        {
            case '_':
                currentRichLine[index].properties |= CharProperty::Underlined;
                break;
            case '-':
                currentRichLine[index].properties |=
                    CharProperty::StrikeThrough;
                break;
            default:
                if (currentRichLine[index].character == ' ')
                {
                    // A space can be overwritten by any other character.
                    currentRichLine[index].character = character;
                }
                break;
        }
    } 
    else if (ignoredCtrlChars.find(character) == ignoredCtrlChars.end())
    {
        switch (character)
        {
            case SO: // Double width mode on.
                currentProps |= CharProperty::DoubleWidth;
                break;
            case DC4: // Double width mode off.
                currentProps &= ~CharProperty::DoubleWidth;
                break;
            case FF: // Add page break.
                currentProps |= CharProperty::PageBreak;
                break;
            case BS: // Back space
                if (currentOverlay.size() > backspaceCount)
                {
                    ++backspaceCount;
                }
                break;
            default:
                //printf("What to do with character 0x%02X\n",
                //       (uint16_t)character);
                break;
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
    if (currentOverlay.size() > 0)
    {
        overlays.push_back(currentOverlay);
    }
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


void PrintOverlayHelper::EvaluateOverlay()
{
    richLine = currentRichLine;
    currentRichLine.clear();
    overlays.clear();
}

void PrintOverlayHelper::EvaluateOverlays()
{
    if (overlays.size() <= 1)
    {
        EvaluateOverlay();
        return;
    }

    richLine.clear();
    currentRichLine.clear();
    auto maxSize = GetMaxOverlaySize();
    if (richLine.size() < maxSize)
    {
        const RichCharacter richChar{ ' ', CharProperty::Normal };

        RichLineAppend(richLine, maxSize - richLine.size(), richChar);
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
                else if (overlay[index] == '-')
                {
                    properties |= CharProperty::StrikeThrough;
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
            properties |= CharProperty::Emphasized;
        }
        else if (count > 3U)
        {
            properties |= CharProperty::DoubleStrike | CharProperty::Emphasized;
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
    currentRichLine.clear();
    currentProps = CharProperty::Normal;
    isEscapeSequence = false;
    escapeSequence.clear();
}

