#pragma author Wolfgang Schwotzer
#pragma description FLEX random file

fn format_hex(u8 value)
{
    return std::format("{} (0x{:02X})", value, value);
};

struct SectorMapEntry
{
    u8 track [[name("Start track"), format("format_hex")]];
    u8 sector [[name("Start sector"), format("format_hex")]];
    u8 count [[name("Number of sectors")]];
} [[format("format_entry")]];

fn format_entry(SectorMapEntry e)
{
    if (e.track == 0 && e.sector == 0 && e.count == 0)
        return "NUL";
    else
        return std::format("{:02X}-{:02X}; {}", e.track, e.sector, e.count);
};

struct SectorMap
{
    SectorMapEntry entry[252 / 3] [[color("0000FF")]];
};

SectorMap sectorMap[2] @ 0;
