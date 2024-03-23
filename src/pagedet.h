/*                                                                              
    pagedet.h


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

#ifndef PAGEDETECTOR_INCLUDED
#define PAGEDETECTOR_INCLUDED

#include "poverhlp.h"
#include "pagedetd.h"


class PageDetector
{
public:
    PageDetector(const RichLines &lines);

    bool HasLinesPerPageDetected() const;
    int16_t GetLinesPerPage() const;

private:
    void EstimateLinesPerPage(PageDetectorData &data);
    void CollectData(PageDetectorData &data);
    void EstimateScore(PageDetectorData &data);
    void EmptyLinesScore(PageDetectorData &data,
                         const std::vector<int16_t> &emptyLines);
    void FirstNonEmptyLinesScore(PageDetectorData &data,
                          const std::map<std::string, int16_t> &nonEmptyLines);
    void NumberOnlyLinesScore(PageDetectorData &data);
    int16_t GetTopEmptyLines(PageDetectorData &data, uint32_t page);
    int16_t GetBottomEmptyLines(PageDetectorData &data, uint32_t page);
    template<typename T>
    static double MeanValue(const std::vector<T> &values);
    template<typename T>
    static double Variance(const std::vector<T> &values);

    void DebugPrint(PageDetectorData &data);

    bool hasLinesPerPageDetected;
    int16_t linesPerPage;
    int verbose;
};

#endif

