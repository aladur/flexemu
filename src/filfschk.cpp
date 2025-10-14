/*
    filfschk.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2025  W. Schwotzer

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
#include "misc1.h"
#include "efiletim.h"
#include "filecntb.h"
#include "filecnts.h"
#include "filfschk.h"
#include "flexerr.h"
#include <cstddef>
#include <string>
#include <algorithm>
#include <array>
#include <set>
#include <vector>
#include <iterator>
#include <sstream>
#include <ostream>
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"


FlexDiskCheck::FlexDiskCheck(
        const IFlexDiskBySector &p_flexDisk,
        FileTimeAccess p_fileTimeAccess) :
    flexDisk(p_flexDisk), fileTimeAccess(p_fileTimeAccess)
{
    Initialize();
}

FlexDiskCheck::~FlexDiskCheck()
= default;

std::string FlexDiskCheck::GetItemName(const item_t &item)
{
    std::string name;

    for (auto ch : item.name)
    {
        if (ch >= ' ' && ch <= '~')
        {
            name += ch;
        }
        else
        {
            std::stringstream stream;

            stream << "\\x" << fmt::format("{:02X}",
                      static_cast<Word>(static_cast<Byte>(ch)));
            name += stream.str();
        }
    }

    return name;
}

std::string FlexDiskCheck::GetItemName(SDWord item_index) const
{
    std::string name = "<unknown>";

    if (item_index >= 0)
    {
        name = GetItemName(items.at(item_index));
    }

    return name;
}

bool FlexDiskCheck::CheckDate(Byte p_day, Byte p_month, Byte p_year)
{
    constexpr static std::array<Byte, 12> max_days{
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    if (p_day == 0 || p_month == 0U || p_month > 12U || p_year > 99U)
    {
        return false;
    }

    Byte max_day = max_days[p_month - 1];
    if (p_month == 2)
    {
        Word year = p_year >= 75U ? p_year + 1900U : p_year + 2000U;
        bool is_leap = (year % 400U == 0U ||
                        (year % 100U != 0U && year % 4U == 0U));
        max_day += (is_leap ? 1U : 0U);
    }

    return p_day <= max_day;
}

bool FlexDiskCheck::CheckTime(Byte p_hour, Byte p_minute)
{
    return p_hour < 24 && p_minute < 60;
}

void FlexDiskCheck::CheckDisk()
{
    if (!CheckDate(disk_day, disk_month, disk_year))
    {
        auto *result = new BadDate;

        result->type = FlexDiskCheckResultItem::Type::Info;
        result->name = "The disk";
        result->day = disk_day;
        result->month = disk_month;
        result->year = disk_year;
        results.emplace_back(result);
    }
}

void FlexDiskCheck::CheckLinks()
{
    for (auto &iter : links)
    {
        const auto &link = iter.second;

        if (link.is_bad)
        {
            for (const auto &previous : link.from)
            {
                auto &previous_link = links.at(previous);
                auto name = GetItemName(previous_link.item_index);
                auto *result = new BadLink;

                result->type =
                    (previous_link.type == SectorType::Free ||
                     previous_link.type == SectorType::Lost) ?
                    FlexDiskCheckResultItem::Type::Warning :
                    FlexDiskCheckResultItem::Type::Error;
                result->name = name;
                result->bad = iter.first;
                result->current = previous;
                results.emplace_back(result);
            }
        }

        if (link.type != SectorType::NotAssigned && link.from.size() > 1)
        {
            auto name = GetItemName(link.item_index);
            auto *result = new MultipleLinkInputs;

            result->type = FlexDiskCheckResultItem::Type::Error;
            result->name = name;
            result->current = iter.first;
            std::copy(link.from.cbegin(), link.from.cend(),
                      std::back_inserter(result->inputs));
            results.emplace_back(result);
        }

        if (link.has_cycle)
        {
            auto *result = new HasCycle;

            result->type = FlexDiskCheckResultItem::Type::Error;
            result->name = GetItemName(link.item_index);
            result->from = iter.first;
            result->back_to = link.to;
            results.emplace_back(result);
        }

        if (!link.is_bad && link.type == SectorType::File &&
            link.record_nr != link.expected_record_nr)
        {
            auto name = GetItemName(link.item_index);
            auto *result = new DiscontiguousRecordNr;

            result->type = FlexDiskCheckResultItem::Type::Warning;
            result->name = name;
            result->current = iter.first;
            result->record_nr = link.record_nr;
            result->expected_record_nr = link.expected_record_nr;
            results.emplace_back(result);
        }
    }
}

void FlexDiskCheck::CheckItems()
{
    for (auto &item : items)
    {
        if (item.type == SectorType::File &&
            !CheckDate(item.day, item.month, item.year))
        {
            auto *result = new BadDate;

            result->type = FlexDiskCheckResultItem::Type::Info;
            result->name = GetItemName(item);
            result->day = item.day;
            result->month = item.month;
            result->year = item.year;
            results.emplace_back(result);
        }

        if (item.type == SectorType::File &&
            fileTimeAccess != FileTimeAccess::NONE &&
            !CheckTime(item.hour, item.minute))
        {
            auto *result = new BadTime;

            result->type = FlexDiskCheckResultItem::Type::Info;
            result->name = GetItemName(item);
            result->hour = item.hour;
            result->minute = item.minute;
            results.emplace_back(result);
        }

        if (item.start == st_t{0, 0} && item.end == st_t{0, 0} &&
            item.type != SectorType::Free)
        {
            auto *result = new NullFile;

            result->type = FlexDiskCheckResultItem::Type::Info;
            result->name = GetItemName(item);
            results.emplace_back(result);

            continue;
        }

        if (item.records != item.sectors &&
            IsTrackSectorValid(item.start) &&
            IsTrackSectorValid(item.end))
        {
            auto *result = new InconsistentRecordSize;

            result->type = FlexDiskCheckResultItem::Type::Warning;
            result->name = GetItemName(item);
            result->records = item.records;
            result->sectors = item.sectors;
            results.emplace_back(result);
        }

        if (IsTrackSectorValid(item.start))
        {
            auto &link = links.at(item.start);

            if (!link.from.empty())
            {
                const auto &inputLink = links.at(*link.from.cbegin());
                auto inputName = GetItemName(inputLink.item_index);
                auto *result = new LinkAndFileInput;

                result->type = FlexDiskCheckResultItem::Type::Error;
                result->inputName = inputName;
                result->input = inputLink.trk_sec;
                result->name = GetItemName(item);
                result->current = link.trk_sec;
                results.emplace_back(result);
            }
        }
        else
        {
            if (item.type != SectorType::Free || item.start != st_t{ 0, 0 })
            {
                auto *result = new BadStart;

                result->type = FlexDiskCheckResultItem::Type::Error;
                result->name = GetItemName(item);
                result->start = item.start;
                results.emplace_back(result);
            }
        }

        if (IsTrackSectorValid(item.end))
        {
             auto &link = links.at(item.end);
             if (link.to != st_t{0, 0})
             {
                auto *result = new LinkAfterEnd;

                result->type =
                    (link.type == SectorType::Free ||
                     link.type == SectorType::Lost) ?
                    FlexDiskCheckResultItem::Type::Warning :
                    FlexDiskCheckResultItem::Type::Error;
                result->name = GetItemName(item);
                result->end = item.end;
                result->to = link.to;
                results.emplace_back(result);
             }
        }
        else
        {
            if (item.type != SectorType::Free || item.start != st_t{ 0, 0 })
            {
                auto *result = new BadEnd;

                result->type = FlexDiskCheckResultItem::Type::Error;
                result->name = GetItemName(item);
                result->end = item.end;
                results.emplace_back(result);
            }
        }
    }

    for (auto &item : items)
    {
        if (item.type == SectorType::Lost)
        {
            auto *result = new LostSectors;

            result->type = FlexDiskCheckResultItem::Type::Info;
            result->name = GetItemName(item);
            result->start = item.start;
            result->end = item.end;
            result->sectors = item.sectors;
            results.emplace_back(result);
        }
    }
}

bool FlexDiskCheck::CheckFileSystem()
{
    CheckDisk();
    CheckLinks();
    CheckItems();

    return IsValid();
}

bool FlexDiskCheck::IsValid() const
{
    return results.empty();
}

const FlexDiskCheckResultItems &FlexDiskCheck::GetResult() const
{
    return results;
}

FlexDiskCheckStatistics_t FlexDiskCheck::GetStatistics() const
{
    FlexDiskCheckStatistics_t statistics;

    statistics[FlexDiskCheckResultItem::Type::Info] = 0;
    statistics[FlexDiskCheckResultItem::Type::Warning] = 0;
    statistics[FlexDiskCheckResultItem::Type::Error] = 0;

    for (const auto &result : results)
    {
        ++statistics[result->type];
    }

    return statistics;
}

std::string FlexDiskCheck::GetStatisticsString() const
{
    auto statistics = GetStatistics();
    std::stringstream stream;
    std::string separator;


    auto value = statistics[FlexDiskCheckResultItem::Type::Error];
    if (value > 0)
    {
        stream << value << " error(s)";
        separator = ", ";
    }
    value = statistics[FlexDiskCheckResultItem::Type::Warning];
    if (value > 0)
    {
        stream << separator << value << " warning(s)";
        separator = " and ";
    }
    value = statistics[FlexDiskCheckResultItem::Type::Info];
    if (value > 0)
    {
        stream << separator << value << " info(s)";
    }

    return stream.str();
}

bool FlexDiskCheck::IsTrackSectorValid(st_t trk_sec) const
{
    // The sectors 00-01, 00-02, 00-03 and 00-04 are rated as invalid
    // because they can not be used as link target.
    return flexDisk.IsTrackValid(trk_sec.trk) &&
           flexDisk.IsSectorValid(trk_sec.trk, trk_sec.sec) &&
           (trk_sec.trk != 0 ||
            (trk_sec.trk == 0 && trk_sec.sec >= first_dir_trk_sec.sec));
}

void FlexDiskCheck::InitializeLinks()
{
    int tracks = 0;
    int sectors = 0;

    flexDisk.GetDiskAttributes(diskAttributes);

    if (!diskAttributes.IsValid())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, flexDisk.GetPath());
    }

    diskAttributes.GetTrackSector(tracks, sectors);

    for (int track = 0; track < tracks; ++track)
    {
        for (int sector = 1; sector <= sectors; ++sector)
        {
            SectorBuffer_t sectorBuffer{};
            st_t current{static_cast<Byte>(track), static_cast<Byte>(sector)};

            if (!IsTrackSectorValid(current) &&
                links.find(current) == links.end())
            {
                links.emplace(current, link_t{current});
                links.at(current).is_bad = true;
                if (track == 0 && sector < first_dir_trk_sec.sec)
                {
                    links.emplace(current, link_t{current});
                    links.at(current).type = SectorType::System;
                }
                continue;
            }

            if (!flexDisk.ReadSector(sectorBuffer.data(), track, sector) &&
                links.find(current) == links.end())
            {
                links.emplace(current, link_t{current});
                links.at(current).is_bad = true;
                continue;
            }

            st_t next{sectorBuffer[0], sectorBuffer[1]};
            auto record_nr = flx::getValueBigEndian<Word>(&sectorBuffer[2]);

            links.emplace(current, link_t{current, next, record_nr});
        }
    }
}

void FlexDiskCheck::AddItem(const std::string &name, SectorType type,
                                 const st_t &start,
                                 const st_t &end /*= st_t{0, 0}*/,
                                 Word records /* = 0 */,
                                 bool is_random /* = false */)
{
    auto current = start;
    st_t previous = {0, 0};
    std::set<st_t> visited;
    auto item_index = static_cast<SDWord>(items.size());
    items.emplace_back(type, start, end, name);
    auto &item = items.at(item_index);
    item.records = records; // record count from directory entry or 0
    item.sectors = 0U; // Sector count according to sector chain
    item.is_random = is_random;
    bool has_end = (end != st_t{0, 0});

    while (current != st_t{0, 0})
    {
        bool is_bad = !IsTrackSectorValid(current);
        if (is_bad && links.find(current) == links.end())
        {
            // Add invalid link if not found.
            links.emplace(current, link_t{current});
        }
        auto &current_link = links.at(current);
        current_link.is_bad = is_bad;

        // Lost sectors: Only follow sector chain if it has not
        // yet been assigned to an item (checked by item_index >= 0).
        if (current_link.type == SectorType::Lost &&
            current_link.item_index >= 0)
        {
            current = st_t{0, 0};
            break;
        }

        if (current_link.type == SectorType::NotAssigned)
        {
            current_link.item_index = item_index;
            current_link.type = type;

            if (type == SectorType::File)
            {
                current_link.expected_record_nr = 0;

                if (item.is_random)
                {
                    // For random files the first two sectors containing the
                    //sector map have record number == 0.
                    if (item.sectors >= 2)
                    {
                        current_link.expected_record_nr = item.sectors - 1;
                    }
                }
                else
                {
                    current_link.expected_record_nr = item.sectors + 1;
                }
            }
        }

        if (previous != st_t{0, 0})
        {
            current_link.from.emplace(previous);
            if (visited.find(current) != visited.end())
            {
                auto &previous_link = links.at(previous);
                previous_link.has_cycle = true;
                current = st_t{0, 0};
                break;
            }
        }

        ++item.sectors;
        visited.emplace(current);

        if (has_end && current == end)
        {
            break;
        }

        if (current_link.is_bad)
        {
            // Bad link found. Set it as end.
            if (has_end)
            {
                item.unexpected_end = current;
            }
            else
            {
                item.end = current;
            }

            break;
        }

        // Goto next link.
        previous = current;
        current = current_link.to;
    }

    if (current == st_t{0, 0})
    {
        if (has_end)
        {
            if (previous != end)
            {
                item.unexpected_end = previous;
            }
        }
        else
        {
            item.end = previous;
        }
    }

    if (item.records == 0)
    {
        item.records = item.sectors;
    }
}

void FlexDiskCheck::InitializeDirectorySectors()
{
    st_t dir_start{0, static_cast<Byte>(first_dir_trk_sec.sec)};

    AddItem("Directory", SectorType::Directory, dir_start);
}

void FlexDiskCheck::InitializeFreeChainSectors()
{
    s_sys_info_sector sis{};

    if (flexDisk.ReadSector(reinterpret_cast<Byte *>(&sis),
                            sis_trk_sec.trk, sis_trk_sec.sec))
    {
        auto fc_start = sis.sir.fc_start;
        auto fc_end = sis.sir.fc_end;
        auto free = flx::getValueBigEndian<Word>(&sis.sir.free[0]);
        disk_day = sis.sir.day;
        disk_month = sis.sir.month;
        disk_year = sis.sir.year;

        AddItem("Free Chain", SectorType::Free, fc_start, fc_end, free);
    }
}

std::string FlexDiskCheck::GetUnixFilename(
        const s_dir_entry &dir_entry)
{
    if (dir_entry.filename[0] != DE_EMPTY &&
        dir_entry.filename[0] != DE_DELETED)
    {
        auto basename(flx::tolower(flx::getstr<>(dir_entry.filename)));
        auto extension((flx::tolower(flx::getstr<>(dir_entry.file_ext))));
        return basename + '.' + extension;
    }

    return {};
}

void FlexDiskCheck::InitializeFileSectors()
{
    st_t current{0, static_cast<Byte>(first_dir_trk_sec.sec)};
    s_dir_sector dir_sector{};

    while (current != st_t{0, 0})
    {
        if (!flexDisk.ReadSector(reinterpret_cast<Byte *>(&dir_sector),
                                 current.trk, current.sec))
        {
            // Directory sector not readable, abort while loop.
            links.emplace(current, link_t{current});
            links.at(current).is_bad = true;
            break;
        }

        for (const auto &dir_entry : dir_sector.dir_entries)
        {
            if (dir_entry.filename[0] == DE_DELETED)
            {
                continue;
            }
            if (dir_entry.filename[0] == DE_EMPTY)
            {
                return;
            }

            auto name = GetUnixFilename(dir_entry);
            auto start = dir_entry.start;
            auto end = dir_entry.end;
            auto records = flx::getValueBigEndian<Word>(&dir_entry.records[0]);
            bool is_random = (dir_entry.sector_map == IS_RANDOM_FILE);
            auto day = dir_entry.day;
            auto month = dir_entry.month;
            auto year = dir_entry.year;
            Byte hour = dir_entry.hour & 0x7FU;
            auto minute = dir_entry.minute;

            AddItem(name, SectorType::File, start, end, records, is_random);
            auto &item = items.at(items.size() - 1);
            item.day = day;
            item.month = month;
            item.year = year;
            item.hour = hour;
            item.minute = minute;
        }

        current = dir_sector.next;
    }
}

void FlexDiskCheck::InitializeLostSectors()
{
    std::vector<st_t> not_assigned;

    for (auto &iter : links)
    {
        const auto &link = iter.second;

        if (link.trk_sec.trk != 0 &&
            link.type == SectorType::NotAssigned &&
            !link.is_bad)
        {
            not_assigned.push_back(link.trk_sec);
            if (IsTrackSectorValid(link.to))
            {
                auto &to_link = links.at(link.to);
                to_link.from.emplace(link.trk_sec);
            }
        }
    }

    int index = 1;
    for (auto &current : not_assigned)
    {
        auto &link = links.at(current);
        if (link.from.empty())
        {
            std::string name = "Lost" + std::to_string(index);

            AddItem(name, SectorType::Lost, link.trk_sec);
            ++index;
        }
    }
}

void FlexDiskCheck::Initialize()
{
    results.clear();
    InitializeLinks();
    InitializeDirectorySectors();
    InitializeFreeChainSectors();
    InitializeFileSectors();
    InitializeLostSectors();
}

void FlexDiskCheck::DumpItemChains(std::ostream &os) const
{
    for (const auto &item : items)
    {
        auto current = item.start;
        bool new_line = false;
        int count = 0;

        os << GetItemName(item) << "\n";
        new_line = true;
        while (current != st_t{0, 0})
        {
            const auto &link = links.at(current);
            if (new_line)
            {
                os << "  ";
            }
            os << " " << current;
            new_line = false;

            if (++count % 12 == 0 || link.is_bad)
            {
                os << (link.is_bad ? "*" : "") << "\n";
                new_line = true;
            }

            if (current == item.end || current == item.unexpected_end)
            {
                break;
            }

            current = link.to;
        }

        if (!new_line)
        {
            os << "\n";
        }
        os << "\n";
    }
}

std::ostream &FlexDiskCheck::DebugDump(std::ostream &os) const
{
    os << "********  I T E M S  ********\n";
    os << "count=" << items.size() << "\n";
    int index = 0;
    for (const auto &item : items)
    {
        os << fmt::format(" {:<3} ", index) << item << "\n";
        ++index;
    }

    os << "********  L I N K S  ********\n";
    os << "count=" << links.size() << "\n";
    for (const auto &iter : links)
    {
        os << " " << iter.second << "\n";
    }

    os << "********  I T E M   C H A I N S  ********\n";
    DumpItemChains(os);

    return os;
}

std::ostream& operator<<(std::ostream &os,
                         FlexDiskCheck::SectorType type)
{
    switch (type)
    {
        case FlexDiskCheck::SectorType::NotAssigned:
            return os << "NotAssigned";
        case FlexDiskCheck::SectorType::System:
            return os << "System";
        case FlexDiskCheck::SectorType::Directory:
            return os << "Directory";
        case FlexDiskCheck::SectorType::Free:
            return os << "Free";
        case FlexDiskCheck::SectorType::File:
            return os << "File";
        case FlexDiskCheck::SectorType::Lost:
            return os << "Lost";
    }

    return os;
}

std::ostream& operator<<(std::ostream &os,
                         const struct FlexDiskCheck::s_link &link)
{
    os << link.trk_sec << " (" << link.type;

    if (!link.from.empty())
    {
        bool is_first = true;
        os << " from=";
        for (const auto &from : link.from)
        {
            if (!is_first)
            {
                os << ",";
            }
            os << from;
            is_first = false;
        }
    }

    if (link.type != FlexDiskCheck::SectorType::System)
    {
        os << " to=" << link.to;
    }

    os << " i_idx=" << link.item_index;

    if (link.type == FlexDiskCheck::SectorType::File)
    {
        os << " rnr=" << link.record_nr;

        if (link.record_nr != link.expected_record_nr)
        {
            os << " exp_rnr=" << link.expected_record_nr;
        }
    }

    if (link.is_bad)
    {
        os << " *bad*";
    }

    if (link.has_cycle)
    {
        os << " *has_cycle*";
    }

    return os << ")";
}

std::ostream &operator<<(std::ostream &os,
                         const FlexDiskCheckResultItemPtr &result)
{
    if (const auto *mli = dynamic_cast<MultipleLinkInputs *>(result.get()))
    {
        return os << *mli;
    }
    if (const auto *lfi = dynamic_cast<LinkAndFileInput *>(result.get()))
    {
        return os << *lfi;
    }
    if (const auto *ef = dynamic_cast<NullFile *>(result.get()))
    {
        return os << *ef;
    }
    if (const auto *bs = dynamic_cast<BadStart *>(result.get()))
    {
        return os << *bs;
    }
    if (const auto *be = dynamic_cast<BadEnd *>(result.get()))
    {
        return os << *be;
    }
    if (const auto *lae = dynamic_cast<LinkAfterEnd *>(result.get()))
    {
        return os << *lae;
    }
    if (const auto *irs =
             dynamic_cast<InconsistentRecordSize *>(result.get()))
    {
        return os << *irs;
    }
    if (const auto *ls = dynamic_cast<LostSectors *>(result.get()))
    {
        return os << *ls;
    }
    if (const auto *hc = dynamic_cast<HasCycle *>(result.get()))
    {
        return os << *hc;
    }
    if (const auto *drn = dynamic_cast<DiscontiguousRecordNr *>(result.get()))
    {
        return os << *drn;
    }
    if (const auto *bl = dynamic_cast<BadLink *>(result.get()))
    {
        return os << *bl;
    }
    if (const auto *bd = dynamic_cast<BadDate *>(result.get()))
    {
        return os << *bd;
    }
    if (const auto *bt = dynamic_cast<BadTime *>(result.get()))
    {
        return os << *bt;
    }
    return os;
}

std::ostream& operator<<(std::ostream &os,
                         const struct FlexDiskCheck::s_item &item)
{
    os << FlexDiskCheck::GetItemName(item) << " (" << item.type << " " <<
          item.start << " " << item.end;
    if (item.unexpected_end != st_t{0, 0})
    {
        os << " unexp=" << item.unexpected_end;
    }
    os << " recs=" << item.records;
    if (item.records != item.sectors)
    {
        os << " secs=" << item.sectors;
    }
    if (item.is_random)
    {
        os << " random";
    }

    return os << ")";
}

std::ostream& operator<<(std::ostream &os, const MultipleLinkInputs &item)
{
    size_t index = 0U;

    os << item.type << "MULIN: " << item.name << " sector " << item.current <<
          " has links from ";
    for (const auto &input : item.inputs)
    {
        if (index + 1 == item.inputs.size())
        {
            os << " and ";
        }
        else if (index > 0U)
        {
            os << ", ";
        }
        ++index;
        os << input;
    }
    return os;
}

std::ostream& operator<<(std::ostream &os, const LinkAndFileInput &item)
{
    return os << item.type << "MULINF: " << item.name << " start sector " <<
                 item.current << " has link from " << item.inputName << " " <<
                 item.input << ".";
}

std::ostream& operator<<(std::ostream &os, const NullFile &item)
{
    return os << item.type << "NULLF: " << item.name << " is a null file.";
}

std::ostream& operator<<(std::ostream &os, const BadStart &item)
{
    return os << item.type << "BADSTA: " << item.name <<
                 " has an bad start sector " << item.start << ".";
}

std::ostream& operator<<(std::ostream &os, const BadEnd &item)
{
    return os << item.type << "BADEND: " << item.name <<
                 " has an bad end sector " << item.end << ".";
}

std::ostream& operator<<(std::ostream &os, const LinkAfterEnd &item)
{
    return os << item.type << "ENDL: End of " << item.name << " " << item.end <<
                 " links to another sector " << item.to << ".";
}

std::ostream& operator<<(std::ostream &os, const InconsistentRecordSize &item)
{
    return os << item.type << "SIZE: " << item.name << " directory records " <<
                 item.records << " is not equal to sector count " <<
                 item.sectors << ".";
}

std::ostream& operator<<(std::ostream &os, const DiscontiguousRecordNr &item)
{
    return os << item.type << "RECNR: " << item.name << " sector " <<
                 item.current << " has record number " <<
                 item.record_nr << ", expected is " <<
                 item.expected_record_nr << ".";
}

std::ostream& operator<<(std::ostream &os, const LostSectors &item)
{
    os << item.type << "LOST: Lost " << item.sectors;
    if (item.sectors == 1)
    {
        return os << " sector, called " << item.name << " at " <<
                     item.start << ".";
    }

    return os << " sectors, called " << item.name << " from " <<
                 item.start << " to " << item.end << ".";
}

std::ostream& operator<<(std::ostream &os, const HasCycle &item)
{
    return os << item.type << "CYCLE: " << item.name << " sector " <<
                 item.from << " has a cycle back to " << item.back_to << ".";
}

std::ostream& operator<<(std::ostream &os, const BadLink &item)
{
    return os << item.type << "BADLNK: " << item.name << " sector " <<
                 item.current << " has an bad link to " << item.bad << ".";
}

std::ostream& operator<<(std::ostream &os, const BadDate &item)
{
    os << item.type <<
          "BADDAT: " << item.name << " has a bad date. MM-DD-YY is " <<
          fmt::format("{:02X}-{:02X}-{:02X}", static_cast<Word>(item.month),
                      static_cast<Word>(item.day),
                      static_cast<Word>(item.year));

    return os;
}

std::ostream& operator<<(std::ostream &os, const BadTime &item)
{
    os << item.type <<
          "BADTIM: " << item.name << " has a bad time. HH-MM is " <<
          fmt::format("{:02X}-{:02X}", static_cast<Word>(item.hour),
                      static_cast<Word>(item.minute));

    return os;
}

std::ostream& operator<<(std::ostream &os, FlexDiskCheckResultItem::Type type)
{
    switch (type)
    {
        case FlexDiskCheckResultItem::Type::Info:
            return os << "I";
        case FlexDiskCheckResultItem::Type::Warning:
            return os << "W";
        case FlexDiskCheckResultItem::Type::Error:
            return os << "E";
    }

    return os;
}

