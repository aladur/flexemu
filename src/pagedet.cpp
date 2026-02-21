/*
    pagedet.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2023-2026  W. Schwotzer

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

#include "misc1.h"
#include "pagedet.h"
#include "pagedetd.h"
#include "poverhlp.h"
#include <cstdint>
#include <cmath>
#include <utility>
#include <numeric>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <iterator>
#include <algorithm>


PageDetector::PageDetector(const RichLines &p_lines)
{
    PageDetectorData data(p_lines);

    EstimateLinesPerPage(data);
}

bool PageDetector::HasLinesPerPageDetected() const
{
    return hasLinesPerPageDetected;
}

int16_t PageDetector::GetLinesPerPage() const
{
    return linesPerPage;
}

void PageDetector::EstimateLinesPerPage(PageDetectorData &data)
{
    if (verbose > 0)
    {
        std::cout << "number of lines=" << data.lines.size() << '\n';
    }
    for (int16_t tryLinesPerPage = 30; tryLinesPerPage < 90; ++tryLinesPerPage)
    {
        data.SetLinesPerPage(tryLinesPerPage);
        CollectData(data);
        EstimateScore(data);
        data.UpdateHighScore();
        if (verbose > 0)
        {
            std::cout << "*** tried linesPerPage=" << tryLinesPerPage <<
            " has a result score=" << data.score << '\n';
        }
    }

    if (data.highScore > 0 && data.fPages > 2.0 &&
            Variance(data.scores) >= 50000.0)
    {
        hasLinesPerPageDetected = true;
        linesPerPage = data.highScoreLinesPerPage;
        if (verbose > 0)
        {
            std::cout << "*** linesPerPage=" << linesPerPage <<
                " scoreVariance=" << Variance(data.scores) << '\n';
        }
    }
    else
    {
        if (verbose > 0)
        {
            std::cout << "*** linesPerPage not detected!" <<
                         " (linesPerPage=" << linesPerPage <<
                         " scoreVariance=" << Variance(data.scores) << ")\n";
        }
    }
}

void PageDetector::CollectData(PageDetectorData &data)
{
    int16_t empty;
    std::string line;

    for (uint32_t page = 0U; page < data.pages; ++page)
    {
        // Check top empty lines and first non-empty line.
        empty = GetTopEmptyLines(data, page);
        data.topEmptyLines.push_back(empty);

        if (data.IsLineValid(page, empty))
        {
            line = data.GetLineString(page, empty);

            auto iter = data.topFirstNonEmptyLines.find(line);
            if (iter == data.topFirstNonEmptyLines.end())
            {
                data.topFirstNonEmptyLines.emplace(line,
                        static_cast<int16_t>(1));
            }
            else
            {
                ++data.topFirstNonEmptyLines[line];
            }
        }

        if (page >= data.pages)
        {
            break;
        }

        // Check bottom empty lines and first non-empty line.
        empty = GetBottomEmptyLines(data, page);
        data.bottomEmptyLines.push_back(empty);
        auto lineOffset = static_cast<int16_t>(-empty - 1);

        if (data.IsLineValid(page + 1U, lineOffset))
        {
            line = data.GetLineString(page + 1, lineOffset);
            auto tLine = flx::trim(line, " ");
            auto isNoDigit = [](char ch){
                // A number-only line may contain numbers, spaces or minus.
                // For example the page number may be printed as:
                // - 5- or -22- which is also detected as number-only line.
                return ((ch < '0' || ch > '9') && ch != '-' && ch != ' ');
            };
            auto tIter = std::find_if(std::begin(tLine), std::end(tLine),
                    isNoDigit);
            if (!tLine.empty() && tIter == std::end(tLine))
            {
                if (verbose > 0)
                {
                    std::cout << "   number line=" << line << '\n';
                }
                ++data.sumBottomLinesWithNumberOnly;
            }
            else
            {
                auto iter = data.bottomFirstNonEmptyLines.find(line);
                if (iter == data.bottomFirstNonEmptyLines.end())
                {
                    data.bottomFirstNonEmptyLines.emplace(line,
                            static_cast<int16_t>(1));
                }
                else
                {
                    ++data.bottomFirstNonEmptyLines[line];
                }
            }
        }
    }
}

int16_t PageDetector::GetTopEmptyLines(PageDetectorData &data, uint32_t page)
{
    int16_t lastValidLineOffset = 0;
    auto pageOffset = page * data.linesPerPage;
    for (int16_t lineOffset = 0; lineOffset < data.linesPerPage; ++lineOffset)
    {
        if (data.IsLineValid(page, lineOffset))
        {
            lastValidLineOffset = lineOffset;
            if (!data.lines[pageOffset + lineOffset].empty())
            {
                return lineOffset;
            }
        }
        else
        {
            ++lastValidLineOffset;
            return lastValidLineOffset;
        }
    }

    return 0;
}

int16_t PageDetector::GetBottomEmptyLines(PageDetectorData &data,
                                          uint32_t page) const
{
    auto pageOffset = page * data.linesPerPage;
    for (auto lineOffset = static_cast<int16_t>(data.linesPerPage - 1);
            lineOffset >= 0; --lineOffset)
    {
        if (data.IsLineValid(page, lineOffset))
        {
            if (!data.lines[pageOffset + lineOffset].empty())
            {
                if (verbose > 0)
                {
                    std::cout << "   bottom pageOffset=" << pageOffset <<
                                 " lineOffset=" << lineOffset << '\n';
                }
                return static_cast<int16_t>(data.linesPerPage - lineOffset - 1);
            }
        }
        else
        {
            return 0;
        }
    }

    return 0;
}

void PageDetector::EstimateScore(PageDetectorData &data)
{
    EmptyLinesScore(data, data.topEmptyLines);
    EmptyLinesScore(data, data.bottomEmptyLines);
    FirstNonEmptyLinesScore(data, data.topFirstNonEmptyLines);
    FirstNonEmptyLinesScore(data, data.bottomFirstNonEmptyLines);
    NumberOnlyLinesScore(data);
    data.UpdateScores();
    if (verbose > 0)
    {
        DebugPrint(data);
    }
}

void PageDetector::EmptyLinesScore(PageDetectorData &data,
                                   const std::vector<int16_t> &emptyLines) const
{
    auto variance = Variance<int16_t>(emptyLines);
    int32_t score = 200;

    if (variance != 0.0)
    {
        score = static_cast<int32_t>(std::fmin(25.0 / variance, 200.0));
    }
    if (verbose > 0)
    {
        std::cout << "   EmptyLinesScore ";
    }
    data.AddToScore(score * static_cast<int32_t>(emptyLines.size()), verbose);
}

void PageDetector::FirstNonEmptyLinesScore(PageDetectorData &data,
        const std::map<std::string, int16_t> &nonEmptyLines) const
{
    for (const auto &iter : nonEmptyLines)
    {
        if (iter.second >= data.GetHalfPageCount())
        {
            if (verbose > 0)
            {
                std::cout << "   FirstNonEmptyLinesScore ";
            }
            data.AddToScore(200 * iter.second, verbose);
            break;
        }
    }
}

void PageDetector::NumberOnlyLinesScore(PageDetectorData &data) const
{
    if (data.sumBottomLinesWithNumberOnly >= data.GetHalfPageCount())
    {
        if (verbose > 0)
        {
            std::cout << "   NumberOnlyLinesScore ";
        }
        data.AddToScore(200 * data.sumBottomLinesWithNumberOnly, verbose);
    }
}

void PageDetector::DebugPrint(PageDetectorData &data)
{
    std::cout << "   fPages=" << data.fPages << '\n';
    std::cout << "   pages=" << data.pages << '\n';

    if (data.sumBottomLinesWithNumberOnly)
    {
        std::cout << "   sumBottomLinesWithNumberOnly=" <<
            data.sumBottomLinesWithNumberOnly << '\n';
    }

    std::cout << "   topEmptyLines";
    std::for_each(std::begin(data.topEmptyLines),
            std::end(data.topEmptyLines), [](int16_t v)
            {
                std::cout << " " << v;
            });
    std::cout << '\n';

    std::cout << "   bottomEmptyLines";
    std::for_each(std::begin(data.bottomEmptyLines),
            std::end(data.bottomEmptyLines), [](int16_t v)
            {
                std::cout << " " << v;
            });
    std::cout << '\n';

    std::cout << "   topFirstNonEmptyLines\n";
    std::for_each(std::begin(data.topFirstNonEmptyLines),
                  std::end(data.topFirstNonEmptyLines),
            [](std::pair<const std::string, int16_t> &p)
            {
                std::cout << "    " << p.first << " " << p.second << '\n';
            });
    std::cout << '\n';

    std::cout << "   bottomFirstNonEmptyLines\n";
    std::for_each(std::begin(data.bottomFirstNonEmptyLines),
                  std::end(data.bottomFirstNonEmptyLines),
            [](std::pair<const std::string, int16_t> &p)
            {
                std::cout << "    " << p.first << " " << p.second << '\n';
            });
}

template<typename T>
double PageDetector::MeanValue(const std::vector<T> &values)
{
    int64_t sum = std::accumulate(values.begin(), values.end(),
            static_cast<int64_t>(0));
    return static_cast<double>(sum) / static_cast<double>(values.size());
}

template<typename T>
double PageDetector::Variance(const std::vector<T> &values)
{
    auto mean = MeanValue<T>(values);
    double sum = 0.0;

    for (T value : values)
    {
        sum += std::pow(static_cast<double>(value) - mean, 2.0);
    };

    return sum / static_cast<double>(values.size());
}

