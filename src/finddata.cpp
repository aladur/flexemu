/*
    finddata.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2025  W. Schwotzer

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


#include "typedefs.h"
#include "finddata.h"
#include "free.h"
#include "findui.h"
#include <cctype>
#include <cassert>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <regex>
#include <exception>
#include <algorithm>


// Initialize for a new Find.
// Find starts to search at offset up to end of data, continues
// from the begin of data up to (excluding) offset.
void FindData::Initialize(DWord startOffset,
        const FindSettingsUi::Config_t &p_findConfig)
{
    findConfig = p_findConfig;
    findStart = startOffset;
    findNext = startOffset;
    isFindSuccess = false;
    isFindDone = false;
    offset = 0U;
}

void FindData::Reset()
{
    findNext = std::nullopt;
}

FindData::Status FindData::FindNext(const std::vector<Byte> &data)
{
    if (!findNext.has_value())
    {
        return Status::NotInitialized;
    }

    FindResult_t result;
    const bool isCase = findConfig.isCaseSensitive;
    const auto &searchFor = findConfig.searchFor;
    const auto oBegin = findNext.value();
    const auto oEnd = (oBegin < findStart) ?
        findStart : static_cast<DWord>(data.size());
    std::vector<Byte> bytes;

    switch (findConfig.searchType)
    {
        case FindSettingsUi::Find::AsciiString:
            if (searchFor.empty())
            {
                return Status::InvalidAsciiString;
            }
            break;

        case FindSettingsUi::Find::RegexString:
            try
            {
                // False positive: variable creation check only is intended.
                // NOLINTNEXTLINE(bugprone-unused-local-non-trivial-variable)
                std::regex test(searchFor, std::regex_constants::extended);
            }
            catch(std::exception &ex)
            {
                errorMessage = ex.what();
                Reset();
                return Status::InvalidRegex;
            }
            break;

        case FindSettingsUi::Find::HexBytes:
            bytes = flx::get_bytes_from_hex(searchFor);
            if (bytes.empty())
            {
                Reset();
                return Status::InvalidHexBytes;
            }
            break;
    }

    if (!isFindDone)
    {
        switch(findConfig.searchType)
        {
            case FindSettingsUi::Find::AsciiString:
                result = FindAscii(data, oBegin, oEnd, searchFor, isCase);
                break;

            case FindSettingsUi::Find::RegexString:
                result = FindRegex(data, oBegin, oEnd, searchFor, isCase);
                break;

            case FindSettingsUi::Find::HexBytes:
                result = FindHex(data, oBegin, oEnd, bytes);
                break;
        }

        if (result.isValid)
        {
            findNext = result.offset + result.size;
            isFindSuccess = true;
            isFindDone = findStart != 0U && findNext.value() == findStart;
            offset = result.offset;
            return Status::FoundItem;
        }

        if (findNext.has_value() && findNext.value() >= findStart)
        {
            findNext = static_cast<DWord>(data.size());
        }

        if (findStart != 0U &&
            findNext.has_value() && findNext.value() > findStart)
        {
            // Nothing more found from findStart till end of data.
            // Continue find starting from offset 0.
            findNext = 0U;
            return FindNext(data);
        }
    }

    Reset();

    return Status::FindFinished;
}

bool FindData::IsFindSuccess() const
{
    return isFindSuccess;
}

bool FindData::IsFindInProgress() const
{
    return findNext != std::nullopt;
}

// If FindNext() returns Status::FoundItem this method returns the
// offset of the found item.
DWord FindData::GetOffset() const
{
    return offset;
}

std::string FindData::GetErrorMessage() const
{
    return errorMessage;
}

static bool caseInsensitiveEqual(char lhs, char rhs)
{
    return std::tolower(lhs) == std::tolower(rhs);
};

FindData::FindResult_t FindData::FindAscii(const std::vector<Byte> &data,
        DWord offsetBegin, DWord offsetEnd, const std::string &text,
        bool isCaseSensitive)
{
    assert(findNext.has_value());

    std::vector<Byte>::const_iterator iter;

    if (isCaseSensitive)
    {
        std::default_searcher searcher(text.cbegin(), text.cend());
        iter = std::search(data.cbegin() + offsetBegin,
                data.cbegin() + offsetEnd, searcher);
    }
    else
    {
        std::default_searcher searcher(text.cbegin(), text.cend(),
            caseInsensitiveEqual);
        iter = std::search(data.cbegin() + offsetBegin,
                data.cbegin() + offsetEnd, searcher);
    }

    if (iter != data.cbegin() + offsetEnd)
    {
        return {
            true,
            static_cast<DWord>(iter - data.cbegin()),
            static_cast<DWord>(text.size())
        };
    }

    return { };
}

FindData::FindResult_t FindData::FindRegex(const std::vector<Byte> &data,
        DWord offsetBegin, DWord offsetEnd, const std::string &regexText,
        bool isCaseSensitive)
{
    assert(findNext.has_value());

    auto flags = std::regex_constants::extended;
    if (!isCaseSensitive)
    {
        flags |= std::regex_constants::icase;
    }
    std::regex regex(regexText, flags);
    std::string_view svdata(reinterpret_cast<const char*>(
                data.data() + offsetBegin), offsetEnd - offsetBegin);
    std::match_results<decltype(svdata)::const_iterator> svmatch;

    if (regex_search(svdata.cbegin(), svdata.cend(), svmatch, regex))
    {
        // regex returns the string found, next find position with FindAscii().
        return FindAscii(data, offsetBegin, offsetEnd, svmatch[0],
                isCaseSensitive);
    }

    return { };
}

FindData::FindResult_t FindData::FindHex(const std::vector<Byte> &data,
        DWord offsetBegin, DWord offsetEnd, const std::vector<Byte> &bytes)
{
    assert(findNext.has_value());

    std::default_searcher searcher(bytes.cbegin(), bytes.cend());
    auto iter = std::search(data.cbegin() + offsetBegin,
            data.cbegin() + offsetEnd, searcher);

    if (iter != data.cbegin() + offsetEnd)
    {
        return {
            true,
            static_cast<DWord>(iter - data.cbegin()),
            static_cast<DWord>(bytes.size())
        };
    }

    return { };
}
