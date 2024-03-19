/*                                                                              
    pagedetd.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2023-2024  W. Schwotzer

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


#include <cmath>
#include <iostream>
#include "pagedetd.h"

PageDetectorData::PageDetectorData(const RichLines &p_lines)
    : lines(p_lines)
    , highScore(0)
    , highScoreLinesPerPage(0)
    , linesPerPage(0)
    , fPages(0.0)
    , pages(0U)
    , score(0)
    , sumBottomLinesWithNumberOnly(0)
{
}

void PageDetectorData::SetLinesPerPage(int16_t p_linesPerPage)
{
    linesPerPage = p_linesPerPage;
    fPages = static_cast<double>(lines.size()) / linesPerPage;
    pages = static_cast<uint32_t>(std::floor(fPages));
    score = 0;
    sumBottomLinesWithNumberOnly = 0;
    topEmptyLines.clear();
    bottomEmptyLines.clear();
    topFirstNonEmptyLines.clear();
    bottomFirstNonEmptyLines.clear();
}

void PageDetectorData::UpdateScores()
{
    if (score > 0)
    {
        scores.push_back(score);
    }
}

void PageDetectorData::UpdateHighScore()
{
    if (score > highScore)
    {
        highScore = score;
        highScoreLinesPerPage = linesPerPage;
    }
}

void PageDetectorData::AddToScore(int32_t scoreOffset, int verbose)
{
    if (verbose > 0)
    {
        std::cout << "AddToScore(" << scoreOffset << ")" << std::endl;
    }
    score += scoreOffset;
}

bool PageDetectorData::IsLineValid(uint32_t page, int16_t lineOffset) const
{
    return page * linesPerPage + lineOffset < lines.size();
}

std::string PageDetectorData::GetLineString(uint32_t page, int16_t lineOffset)
                                                                          const
{
    std::string result;

    if (IsLineValid(page, lineOffset))
    {
        result = toString(lines[page * linesPerPage + lineOffset]);
    }

    return result;
}

int32_t PageDetectorData::GetHalfPageCount() const
{
    return static_cast<int32_t>(std::ceil(static_cast<float>(pages) / 2.0F));
}

