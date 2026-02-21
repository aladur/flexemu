/*
    finddata.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2025-2026  W. Schwotzer

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

#ifndef FINDDATA_INCLUDED
#define FINDDATA_INCLUDED

#include "typedefs.h"
#include "findui.h"
#include <string>
#include <vector>
#include <optional>


class FindData
{
public:

    // Status returned from FindNext()
    enum class Status : uint8_t
    {
        FoundItem, // An item has been found
        FindFinished, // Find is now finished
        NotInitialized, // Error: Initialize() has not been called
        InvalidHexBytes, // Error: Hex bytes contains invalid data
        InvalidRegex, // Error: Regular expression is invalid
        InvalidAsciiString, // Error: Ascii string is invalid
    };

    void Reset();
    void Initialize(DWord startOffset,
            const FindSettingsUi::Config_t &p_findConfig);
    Status FindNext(const std::vector<Byte> &data);
    bool IsFindSuccess() const;
    bool IsFindInProgress() const;
    DWord GetOffset() const;
    std::string GetErrorMessage() const;

protected:

    struct sFindResult
    {
        bool isValid{};
        DWord offset{};
        DWord size{};
    };

    using FindResult_t = struct sFindResult;

    FindResult_t FindAscii(const std::vector<Byte> &data, DWord offsetBegin,
            DWord offsetEnd, const std::string &text, bool isCaseSensitive);
    FindResult_t FindRegex(const std::vector<Byte> &data, DWord offsetBegin,
            DWord offsetEnd, const std::string &regexText,
            bool isCaseSensitive);
    FindResult_t FindHex(const std::vector<Byte> &data, DWord offsetBegin,
            DWord offsetEnd, const std::vector<Byte> &bytes);

private:
    FindSettingsUi::Config_t findConfig;
    std::string errorMessage;
    DWord findStart{};
    DWord offset{};
    std::optional<DWord> findNext;
    bool isFindSuccess{};
    bool isFindDone{};
};
#endif
