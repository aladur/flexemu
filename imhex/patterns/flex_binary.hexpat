#pragma author Wolfgang Schwotzer
#pragma description FLEX binary file (*.cmd; *.bin; *.sys; ...)
#pragma endian big

import std.string;
import std.mem;

fn has_more_NUL(u32 size)
{
    bool has_more = $ < size;
    if (has_more)
    {
        has_more = std::mem::read_unsigned($, 1) == 0;
        $ = $ - 1;
    }
    return has_more;
};

fn format_addr(u16 addr)
{
    return std::format("{:04X}", addr);
};


enum RecordType : u8
{
    Data = 0x02,
    StartAddr = 0x16,
    NUL = 0x00
};

struct DataRecord
{
    u16 address [[name("Address"), color("3F3FFF"), format("format_addr")]];
    u8 size [[name("Data size"),color("007F00")]];
    u8 data[size] [[color("00FF00")]];
} [[name("A Data Record")]];

struct StartAddrRecord
{
    u16 address [[name("Start addr."), format("format_addr")]];
};

struct NULRecord
{
    u8 stuffing[while(has_more_NUL(std::mem::size()))] [[color("7F7F7F")]];;
};

struct Record
{
    RecordType type [[color("FF0000")]];

    match(type)
    {
        (RecordType::Data): DataRecord record;
        (RecordType::StartAddr): StartAddrRecord record;
        (RecordType::NUL): NULRecord record;
    }
} [[format("format_record_type")]];

fn format_record_type(Record r)
{
    if (r.type == RecordType::Data)
    {
        return std::format("Data[{:04X}:{:04X}]", r.record.address, r.record.address + r.record.size - 1);
    }

    else if (r.type == RecordType::StartAddr)
    {
        return std::format("StartAddr[{:04X}]", r.record.address);
    }

    else if (r.type == RecordType::NUL)
    {
        return "NUL";
    }
};

Record records[while($ < std::mem::size())] @ $;
