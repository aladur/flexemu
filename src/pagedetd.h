/*                                                                              
    pagedetd.h


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

#ifndef PAGEDETECTORDATA_INCLUDED
#define PAGEDETECTORDATA_INCLUDED

#include <string>
#include <vector>
#include <map>
#include "poverhlp.h"


struct PageDetectorData
{
    PageDetectorData() = delete;
    PageDetectorData(const RichLines &p_lines);

    void SetLinesPerPage(uint32_t p_linesPerPage);
    bool IsLineValid(uint32_t page, int32_t lineOffset) const;
    std::string GetLineString(uint32_t page, int32_t lineOffset) const;
    void AddToScore(int32_t scoreOffset, int verbose);
    uint32_t GetHalfPageCount() const;
    void UpdateScores();
    void UpdateHighScore();

    const RichLines &lines;
    int32_t highScore;
    int32_t highScoreLinesPerPage;

    uint32_t linesPerPage;
    double fPages;
    uint32_t pages;
    int32_t score;
    std::vector<int32_t> scores;
    uint32_t sumBottomLinesWithNumberOnly;
    std::vector<uint32_t> topEmptyLines;
    std::vector<uint32_t> bottomEmptyLines;
    std::map<std::string, uint32_t> topFirstNonEmptyLines;
    std::map<std::string, uint32_t> bottomFirstNonEmptyLines;
};

#endif

