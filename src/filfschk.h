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

#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include "typedefs.h"
#include "efiletim.h"
#include "filecnts.h"
#include "fcinfo.h"


struct FlexDiskCheckResultItem
{
    enum class Type : uint8_t
    {
        Info,
        Warning,
        Error,
    };

    Type type{};

    virtual ~FlexDiskCheckResultItem() = default;

    friend std::ostream& operator<<(std::ostream &os,
                                    FlexDiskCheckResultItem::Type type);
};

using FlexDiskCheckResultItemPtr = std::unique_ptr<FlexDiskCheckResultItem>;
using FlexDiskCheckResultItems = std::vector<FlexDiskCheckResultItemPtr>;
using FlexDiskCheckStatistics_t = std::map<FlexDiskCheckResultItem::Type, int>;

class FlexDiskCheck
{
    enum class SectorType : uint8_t
    {
        NotAssigned = 0U,
        System,
        Directory,
        Free,
        File,
        Lost,
    };

    struct s_link
    {
        SectorType type{SectorType::NotAssigned};
        st_t trk_sec{}; // track-sector of this sector.
        st_t to{}; // track-sector to next linked sector or 00-00.
        std::set<st_t> from; // track-sector from previous linked sector(s)
        SDWord item_index{-1}; // index of according item
        Word record_nr{0}; // record number
        Word expected_record_nr{0}; // expected record number (only valid if
                                    // type == SectorType::File)
        bool is_bad{false}; // flag if trk_sec is a bad track-sector
                            // (not usable as link).
        bool has_cycle{false}; // flag if this link has a cycle

        s_link() = default;

        explicit s_link(st_t ts, st_t p_to = st_t{0, 0}, Word p_record_nr = 0) :
              trk_sec(ts), to(p_to), record_nr(p_record_nr)
        {
        }
    };
    using link_t = struct s_link;

    struct s_item
    {
        SectorType type{SectorType::NotAssigned};
        std::string name;
        st_t start{}; // Start track-sector (For Files part of directory entry)
        st_t end{}; // End track-sector (For Files part of directory entry)
        st_t unexpected_end{}; // For Files: End track-sector (different from end)
        Word records{0};
        Word sectors{0};
        bool is_random{false};
        Byte month{0};
        Byte day{0};
        Byte year{0};
        Byte hour{0};
        Byte minute{0};

        s_item(SectorType p_type, st_t p_start, st_t p_end, std::string p_name)
            : type(p_type)
            , name(std::move(p_name))
            , start(p_start)
            , end(p_end)
        {
        };
    };
    using item_t = struct s_item;

public:
    FlexDiskCheck() = delete;
    FlexDiskCheck(
            const IFlexDiskBySector &p_flexDisk,
            FileTimeAccess fileTimeAccess);
    FlexDiskCheck(const FlexDiskCheck &src) = delete;
    FlexDiskCheck(FlexDiskCheck &&src) = delete;
    ~FlexDiskCheck();

    FlexDiskCheck &operator= (const FlexDiskCheck &src) = delete;
    FlexDiskCheck &operator= (FlexDiskCheck &&src) = delete;

    bool CheckFileSystem();
    bool IsValid() const;
    const FlexDiskCheckResultItems &GetResult() const;
    FlexDiskCheckStatistics_t GetStatistics() const;
    std::string GetStatisticsString() const;
    std::ostream &DebugDump(std::ostream &os) const;

private:
    void Initialize();
    void InitializeLinks();
    void InitializeDirectorySectors();
    void InitializeFreeChainSectors();
    void InitializeFileSectors();
    void InitializeLostSectors();

    void CheckDisk();
    void CheckLinks();
    void CheckItems();
    static bool CheckDate(Byte day, Byte month, Byte year);
    static bool CheckTime(Byte hour, Byte minute);
    void DumpItemChains(std::ostream &os) const;

    void AddItem(const std::string &name, SectorType type,
                 const st_t &start,
                 const st_t &end = st_t{0, 0},
                 Word records = 0,
                 bool is_random = false);

    static std::string GetItemName(const item_t &item);
    std::string GetItemName(SDWord item_index) const;
    bool IsTrackSectorValid(st_t trk_sec) const;
    static std::string GetUnixFilename(const s_dir_entry &dir_entry);

    friend std::ostream& operator<<(std::ostream &os,
                                    FlexDiskCheck::SectorType type);
    friend std::ostream& operator<<(std::ostream &os,
                       const struct FlexDiskCheck::s_link &link);
    friend std::ostream& operator<<(std::ostream &os,
                       const struct FlexDiskCheck::s_item &item);

    const IFlexDiskBySector &flexDisk;
    FlexDiskAttributes diskAttributes;
    std::map<st_t, link_t> links;
    std::vector<item_t> items;
    FlexDiskCheckResultItems results;
    Byte disk_month{0};
    Byte disk_day{0};
    Byte disk_year{0};
    FileTimeAccess fileTimeAccess{FileTimeAccess::NONE};
};

// The following objects represent and discribe the check results.

struct MultipleLinkInputs : public FlexDiskCheckResultItem
{
    std::string name;
    st_t current{}; // Current track-sector with multiple inputs.
    std::vector<st_t> inputs; // Input Track-sector's having a link to current.
};

struct LinkAndFileInput : public FlexDiskCheckResultItem
{
    std::string name;
    st_t current{};
    std::string inputName; // Item name with start track-sector to current
    st_t input{}; // Track-sector with link to current.
};

struct NullFile : FlexDiskCheckResultItem
{
    std::string name;
};

struct BadStart : FlexDiskCheckResultItem
{
    std::string name;
    st_t start{};
};

struct BadEnd : FlexDiskCheckResultItem
{
    std::string name;
    st_t end{};
};

struct LinkAfterEnd : FlexDiskCheckResultItem
{
    std::string name;
    st_t end{}; // Track-Sector marked as end
    st_t to{}; // Track-Sector to which end sector links to.
};

struct InconsistentRecordSize : FlexDiskCheckResultItem
{
    std::string name;
    Word records{0}; // Number of records in directory entry.
    Word sectors{0}; // Number of sectors according to sector chain.
};

struct DiscontiguousRecordNr : FlexDiskCheckResultItem
{
    std::string name; // Item name
    st_t current{}; // current track-sector
    Word record_nr{}; // actual record number
    Word expected_record_nr{}; // expected record number
};

struct LostSectors : FlexDiskCheckResultItem
{
    std::string name;
    st_t start{};
    st_t end{};
    Word sectors{}; // Number of sectors.
};

struct HasCycle : FlexDiskCheckResultItem
{
    std::string name;
    st_t from{}; // Track-sector which has a link back to back_to.
    st_t back_to{};
};

struct BadLink : FlexDiskCheckResultItem
{
    std::string name;
    st_t bad{}; // Bad track-sector.
    st_t current{}; // Track-sector with the bad link.
};

struct BadDate : FlexDiskCheckResultItem
{
    std::string name;
    Byte day{};
    Byte month{};
    Byte year{};
};

struct BadTime : FlexDiskCheckResultItem
{
    std::string name;
    Byte hour{};
    Byte minute{};
};

extern std::ostream &operator<<(std::ostream &os,
                                const FlexDiskCheckResultItemPtr &result);

extern std::ostream& operator<<(std::ostream &os,
                                const MultipleLinkInputs &item);
extern std::ostream& operator<<(std::ostream &os, const LinkAndFileInput &item);
extern std::ostream& operator<<(std::ostream &os, const NullFile &item);
extern std::ostream& operator<<(std::ostream &os, const BadStart &item);
extern std::ostream& operator<<(std::ostream &os, const BadEnd &item);
extern std::ostream& operator<<(std::ostream &os, const LinkAfterEnd &item);
extern std::ostream& operator<<(std::ostream &os,
                                const InconsistentRecordSize &item);
extern std::ostream& operator<<(std::ostream &os,
                                const DiscontiguousRecordNr &item);
extern std::ostream& operator<<(std::ostream &os, const LostSectors &item);
extern std::ostream& operator<<(std::ostream &os, const HasCycle &item);
extern std::ostream& operator<<(std::ostream &os, const BadLink &item);
extern std::ostream& operator<<(std::ostream &os, const BadDate &item);
extern std::ostream& operator<<(std::ostream &os, const BadTime &item);

