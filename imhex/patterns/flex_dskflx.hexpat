#pragma author Wolfgang Schwotzer
#pragma description FLEX disk image file (*.dsk; *.flx; *.wta)
#pragma endian big
#pragma pattern_limit 2088960
#pragma loop_limit 6000
#pragma MIME application/x-flex-diskimage
#pragma MIME application/x-flex-flxdiskimage

import std.string;
import std.mem;

// constant definitions
#define MAX_DIR_SECTORS 6000
#define SECTOR_SIZE 256
#define FLEX_FILENAME_LENGTH 8
#define FLEX_FILEEXT_LENGTH 3
#define FLX_MAGIC_NUMBER 0x339A5C48

// global variables
u16 dir_sectors[MAX_DIR_SECTORS]; // Contains hashes for each dir. sector.
u16 dir_sectors_index = 0; // Next free index into dir_sectors array.
u8 max_track = 0; // Maximum track number. (0 based).
u8 max_sector0 = 0; // Maximum sector number on track 0 (1 based).
u8 max_sector = 0; // Maximum sector number on tracks > 0 (1 based).
u16 byte_per_track0 = 0; // Total bytes on track 0.
u16 byte_per_track = 0; // Total bytes on tracks > 0.
u8 disk_offset = 0; // Size of file header (JVC or FLX), or 0.

// global in/out variables
u16 o_track_count out; // Total track count (1 based).
u16 o_sector_count out; // Total sector count (1 based).
u16 o_dir_sectors out; // Number of detected directory sectors.

bitfield b_attributes
{
    FLX_READONLY : 1;
    FLX_NODELETE : 1;
    FLX_NOREAD : 1;
    FLX_NOCAT : 1;
};

struct st_t
{
    u8 track [[format("format_hex")]];
    u8 sector [[format("format_hex")]];
};

// Optionally a JVC header can be specified at the beginning of the disk.
// It may have a size of 1 ... 5 byte.
struct jvc_header1
{
    u8 sector_per_track; // number of sectors per track
};
struct jvc_header2 : jvc_header1
{
    u8 sides; // number of sides on a disk.
};
struct jvc_header3 : jvc_header2
{
    u8 size_code; // should be 1, for 256 byte per sector.
};
struct jvc_header4 : jvc_header3
{
    u8 sector_id; // id of first sector, should be 1.
};
struct jvc_header5 : jvc_header4
{
    u8 sector_attr_flag; // sector attributes, should be 0.
};

// File header of a *.flx disk image file.
struct s_flex_header
{
    /* magic number to identify right format, big endian format */
    u32 magic_number [[format("format_magic_number")]];
    u8 write_protect; /* if != 0 disk image is write-prot*/
    u8 sizecode; /* 128 * 2 ^ n Bytes per Sector */
    u8 sides0; /* nr. of sides on track 0 */
    u8 sectors0; /* nr. of sect. on track 0 (one side)*/
    u8 sides; /* nr. of sides on track != 0 */
    u8 sectors; /* nr. of sectors on track != 0 */
    u8 tracks; /* nr. of tracks total */
    u8 dummy1; /* for future extension */
    u8 dummy2, dummy3, dummy4, dummy5; /* and for alignment 4*/
};

// FLEX struct definitions.
// If the struct is used in flexemu use the same struct name.
struct boot_sector
{
    u8 code[SECTOR_SIZE];
} [[format("format_boot_sector"), color("7F007F")]];

struct s_sys_info_record
{
    char disk_name[FLEX_FILENAME_LENGTH];
    char disk_ext[FLEX_FILEEXT_LENGTH];
    u16 disk_number;
    st_t fc_start [[name("FreeChain Start"), format("format_st")]];
    st_t fc_end [[name("FreeChain End"), format("format_st")]];
    u16 free;
    u8 month;
    u8 day;
    u8 year [[format("format_year")]];
    st_t last [[name("Last trk-sec"), format("format_st")]];
};

struct s_sys_info_sector
{
    u8 unused1[16];
    s_sys_info_record sir;
    u8 unused2[216];
} [[format("format_s_sys_info_sector"), color("007F7F")]];

// A directory entry represents one file in the disk directory.
// It contains the filename, file extension and several file
// properties.
struct s_dir_entry
{
    char filename[FLEX_FILENAME_LENGTH];
    char file_ext[FLEX_FILEEXT_LENGTH];
    b_attributes file_attr;
    u8 hour;
    st_t start [[name("Start"), format("format_st")]];
    st_t end [[name("End"), format("format_st")]];
    u16 records;
    u8 sector_map [[format("format_sector_map")]];
    u8 minute;
    u8 month;
    u8 day;
    u8 year [[format("format_year")]];
};

// A directory sector contains up to 10 directory entries.
struct s_dir_sector
{
    st_t next [[name("Next"), format("format_st")]];
    u16 record_nr;
    u8 unused[12];
    s_dir_entry dir_entries[10];
} [[format("format_s_dir_sector"), color("007F00")]];

// A data sector is any sector in the free chain or used as part of a file.
struct data_sector
{
    st_t next [[name("Next"), format("format_st")]];
    u16 record_nr;
    u8 data[SECTOR_SIZE - 4];
} [[format("format_data_sector"), color("00007F")]];

struct sector
{
    if (is_dir_sector($))
        s_dir_sector directory;
    else
        data_sector data;
} [[inline]];

// Function definitions.
//
// The FLEX directory sectors need special treatment. There is an array
// containing a hash for each directory sector. The funtion is_dir_sector
// returns true if it is a directory sector. The functions
//   lower_bound()
//   binary_search()
//   insert()
//   initialize_dir_sector_hashes()
//   is_dir_sector()
// are related to this directory sector treatment.

fn lower_bound(u16 hash)
{
    s16 low = 0;
    s16 high = dir_sectors_index - 1;
    s16 result = -1;

    while (low <= high)
    {
        s16 mid = low + ((high - low) >> 1);
        if (dir_sectors[mid] < hash)
        {
            result = mid;
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }
    return result + 1;
};

fn binary_search(u16 hash)
{
    return (dir_sectors[lower_bound(hash)] == hash);
};

fn insert(u16 index, u16 hash)
{
    if (index > dir_sectors_index || dir_sectors_index >= MAX_DIR_SECTORS)
    {
        return; // index out of valid range or array is full. ignore.
    }

    u16 i = dir_sectors_index;
    while (i != index)
    {
        dir_sectors[i] = dir_sectors[i - 1];
        i -= 1;
    }
    dir_sectors[index] = hash;
    dir_sectors_index += 1;
};

fn initialize_dir_sector_hashes()
{
    // Create a hash list for all sectors which have been identified as
    // a directory sector. This allows to quickly lookup for each sector
    // if it is a data sector or directory sector.
    st_t previous;
    st_t next;

    next.track = 0; // Next track for directory sector chain.
    next.sector = 5; // Next sector for directory sector chain.

    // Initialize dir_sectors by reading along the directory sector
    // chain.
    while (next.track != 0 || next.sector != 0)
    {
        u16 hash = (next.track << 8) + next.sector;
        u16 index = lower_bound(hash);
        if (dir_sectors[index] == hash)
        {
            str message = std::format(
                "Directory sector {} links back to {}.",
                format_st(previous), format_st(next));
            std::warning(message);
            // Abort reading directory chain.
            next.track = 0;
            next.sector = 0;
            continue;
        }
        insert(index, hash);
        previous = next;
        u32 pos;
        if (next.track == 0)
            pos = disk_offset + (next.sector - 1) * SECTOR_SIZE;
        else
            pos = disk_offset + byte_per_track0 +
                ((next.track - 1) * byte_per_track) +
                (next.sector - 1) * SECTOR_SIZE;
        next.track = $[pos];
        next.sector = $[pos + 1];
        if (next.track > max_track || next.sector > max_sector ||
            (next.track != 0 && next.sector == 0))
        {
            str message = std::format(
                "Directory sector {} links to an invalid sector {}.",
                format_st(previous), format_st(next));
            std::warning(message);
            // Abort reading directory chain.
            next.track = 0;
            next.sector = 0;
        }
    }
};

fn is_dir_sector(u32 pos)
{
    if (max_track == 0)
    {
        // should be: max_track = fulldisk.disk.sis[0].sir.last.track;
        max_track = $[550 + disk_offset];
    }

    if (max_sector == 0)
    {
        // should be: max_sector = fulldisk.disk.sis[0].sir.last.sector;
        max_sector = $[551 + disk_offset];
        max_sector0 = max_sector0;
        byte_per_track = max_sector * SECTOR_SIZE;
        byte_per_track0 = byte_per_track;
    }

    if (dir_sectors_index == 0)
    {
        initialize_dir_sector_hashes();
    }

    st_t ts = calculate_ts($);
    u16 hash = (ts.track << 8) + ts.sector;

    if (ts.track == max_track && ts.sector == max_sector)
    {
        // All sectors have been parsed. Finally set some output variables.
        o_dir_sectors = dir_sectors_index;
        o_sector_count = max_sector;
        o_track_count = max_track + 1;
    }

    return binary_search(hash);
};

// Format helper functions.

fn format_hex(u8 value)
{
    return std::format("{} (0x{:02X})", value, value);
};

fn format_st(st_t st)
{
    return std::format("{:02X}-{:02X}", st.track, st.sector);
};

fn format_year(u8 year)
{
    if (year < 75)
        return std::format("{}", year + 2000);
    else
        return std::format("{}", year + 1900);
};

fn format_magic_number(u32 magic_number)
{
    return std::format("{:08X}", magic_number);
};

fn format_sector_map(u8 sector_map)
{
    if (sector_map != 0)
        return "IS_RANDOM";
    else
        return "";
};

fn calculate_ts(u32 pos)
{
    // Create a track-sector struct (st_t) from file position pos.
    st_t ts;

    pos -= disk_offset;
    if (pos >= byte_per_track0)
    {
        u32 pos1 = pos - byte_per_track0;
        ts.track = 1 + (pos1 / byte_per_track);
        ts.sector = 1 + ((pos1 % byte_per_track) / SECTOR_SIZE);
    }
    else
    {
        ts.track = 0;
        ts.sector = 1 + (pos / SECTOR_SIZE);
    }

    return ts;
};

fn calculate_ts_string(u32 pos)
{
    // Create a track-sector string from file position pos.
    return format_st(calculate_ts(pos));
};

fn format_boot_sector(boot_sector boot)
{
    return std::format("boot sector {}", calculate_ts_string($));
};

fn format_s_sys_info_sector(s_sys_info_sector sys)
{
    return std::format("sys.info s. {}", calculate_ts_string($));
};

fn format_s_dir_sector(s_dir_sector s)
{
    return std::format("dir. sector {}", calculate_ts_string($));
};

fn format_data_sector(data_sector d)
{
    return std::format("data sector {}", calculate_ts_string($));
};

struct Disk
{
    boot_sector boot[2];
    s_sys_info_sector sis[2];
    sector sector[while($ < std::mem::size())];
};

struct DiskWithPrefix
{
    disk_offset = std::mem::size() % SECTOR_SIZE;

    if (disk_offset == 1)
    {
        jvc_header1 jvc_header;
    }
    else if (disk_offset == 2)
    {
        jvc_header2 jvc_header;
    }
    else if (disk_offset == 3)
    {
        jvc_header3 jvc_header;
    }
    else if (disk_offset == 4)
    {
        jvc_header4 jvc_header;
    }
    else if (disk_offset == 5)
    {
        jvc_header5 jvc_header;
    }
    else if (disk_offset == 16)
    {
        s_flex_header flx_header;

        if (FLX_MAGIC_NUMBER != flx_header.magic_number)
        {
            std::error("FLX file header has unknown magic number. Aborted.");
        }
        if (1 != flx_header.sizecode)
        {
            str message = std::format(
                "FLX file header has unsupported sizecode {}. Aborted.",
                flx_header.sizecode);
            std::error(message);
        }

        max_sector0 = flx_header.sides0 * flx_header.sectors0;
        max_sector = flx_header.sides * flx_header.sectors;
        max_track = flx_header.tracks - 1;
        byte_per_track0 = max_sector0 * SECTOR_SIZE;
        byte_per_track = max_sector * SECTOR_SIZE;
    }

    Disk disk;
} [[inline]];

DiskWithPrefix fulldisk @ 0;

