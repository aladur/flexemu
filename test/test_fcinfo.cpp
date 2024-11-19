#include "gtest/gtest.h"
#include "fcinfo.h"
#include "filecntb.h"
#include <iostream>


TEST(test_fcinfo, default_ctor)
{
    int tracks = 0;
    int sectors = 0;
    FlexDiskAttributes info;
    EXPECT_TRUE(info.GetPath().empty());
    info.GetTrackSector(tracks, sectors);
    EXPECT_EQ(tracks, 0);
    EXPECT_EQ(sectors, 0);
    EXPECT_TRUE(info.GetName().empty());
    EXPECT_EQ(info.GetNumber(), 0U);
    EXPECT_EQ(info.GetType(), DiskType::DSK);
    EXPECT_EQ(info.GetTypeString(), "Disk image file, DSK format");
    EXPECT_EQ(info.GetOptions(), DiskOptions::NONE);
    EXPECT_EQ(info.GetFree(), 0U);
    EXPECT_EQ(info.GetTotalSize(), 0U);
    EXPECT_EQ(info.GetAttributes(), 0U);
    EXPECT_FALSE(info.GetIsFlexFormat());
    EXPECT_FALSE(info.GetIsWriteProtected());
    EXPECT_FALSE(info.IsValid());
    EXPECT_TRUE(info.GetJvcFileHeader().empty());
}

TEST(test_fcinfo, copy_ctor)
{
    FlexDiskAttributes info_src;
    info_src.SetPath("/tmp/disk.dsk");
    info_src.SetName("abc");
    auto info_tgt(info_src);
    EXPECT_EQ(info_src.GetPath(), "/tmp/disk.dsk");
    EXPECT_EQ(info_src.GetName(), "abc");
    EXPECT_TRUE(info_src.IsValid());
    EXPECT_EQ(info_tgt.GetPath(), "/tmp/disk.dsk");
    EXPECT_EQ(info_tgt.GetName(), "abc");
    EXPECT_TRUE(info_tgt.IsValid());
}

TEST(test_fcinfo, move_ctor)
{
    FlexDiskAttributes info_src;
    info_src.SetPath("/tmp/disk.dsk");
    info_src.SetName("abc");
    auto info_tgt(std::move(info_src));
    EXPECT_TRUE(info_src.GetPath().empty());
    EXPECT_TRUE(info_src.GetName().empty());
    EXPECT_TRUE(info_src.IsValid());
    EXPECT_EQ(info_tgt.GetPath(), "/tmp/disk.dsk");
    EXPECT_EQ(info_tgt.GetName(), "abc");
    EXPECT_TRUE(info_tgt.IsValid());
}

TEST(test_fcinfo, copy_assignment)
{
    FlexDiskAttributes info_src;
    info_src.SetPath("/tmp/disk.dsk");
    info_src.SetName("abc");
    auto info_tgt = info_src;
    EXPECT_EQ(info_src.GetPath(), "/tmp/disk.dsk");
    EXPECT_EQ(info_src.GetName(), "abc");
    EXPECT_TRUE(info_src.IsValid());
    EXPECT_EQ(info_tgt.GetPath(), "/tmp/disk.dsk");
    EXPECT_EQ(info_tgt.GetName(), "abc");
    EXPECT_TRUE(info_tgt.IsValid());
}

TEST(test_fcinfo, move_assignment)
{
    FlexDiskAttributes info_src;
    info_src.SetPath("/tmp/disk.dsk");
    info_src.SetName("abc");
    auto info_tgt = std::move(info_src);
    EXPECT_TRUE(info_src.GetPath().empty());
    EXPECT_TRUE(info_src.GetName().empty());
    EXPECT_TRUE(info_src.IsValid());
    EXPECT_EQ(info_tgt.GetPath(), "/tmp/disk.dsk");
    EXPECT_EQ(info_tgt.GetName(), "abc");
    EXPECT_TRUE(info_tgt.IsValid());
}


TEST(test_fcinfo, get_set)
{
    FlexDiskAttributes info;
    info.SetDate(BDate(28, 5, 1985));
    EXPECT_EQ(info.GetDate(), BDate(28, 5, 1985));
    info.SetPath("dir/disk.dsk");
    EXPECT_EQ(info.GetPath(), "dir/disk.dsk");
    info.SetName("testname");
    EXPECT_EQ(info.GetName(), "testname");
    info.SetNumber(4711);
    EXPECT_EQ(info.GetNumber(), 4711);
    info.SetTrackSector(22, 47);
    int tracks = 0;
    int sectors = 0;
    info.GetTrackSector(tracks, sectors);
    EXPECT_EQ(tracks, 22);
    EXPECT_EQ(sectors, 47);
    info.SetFree(8255);
    EXPECT_EQ(info.GetFree(), 8255);
    info.SetTotalSize(897445);
    EXPECT_EQ(info.GetTotalSize(), 897445);
    info.SetIsFlexFormat(true);
    EXPECT_TRUE(info.GetIsFlexFormat());
    info.SetIsWriteProtected(true);
    EXPECT_TRUE(info.GetIsWriteProtected());
    std::vector<Byte> jvcHeader{0x20, 0x01, 0x00};
    info.SetJvcFileHeader(jvcHeader);
    EXPECT_EQ(info.GetJvcFileHeader().size(), 3U);
    EXPECT_EQ(info.GetJvcFileHeader(), jvcHeader);
    EXPECT_TRUE(info.IsValid());
    auto type = DiskType::Directory;
    info.SetType(type);
    EXPECT_EQ(info.GetType(), type);
    EXPECT_EQ(info.GetTypeString(), "directory");
    type = DiskType::DSK;
    info.SetType(type);
    EXPECT_EQ(info.GetType(), type);
    EXPECT_EQ(info.GetTypeString(), "Disk image file, DSK format");
    type = DiskType::FLX;
    info.SetType(type);
    EXPECT_EQ(info.GetType(), type);
    EXPECT_EQ(info.GetTypeString(), "Disk image file, FLX format");
    auto options = DiskOptions::NONE;
    info.SetOptions(options);
    EXPECT_EQ(info.GetOptions(), options);
    options |= DiskOptions::JvcHeader;
    info.SetOptions(options);
    EXPECT_EQ(info.GetOptions(), options);
    options |= DiskOptions::HasSectorIF | DiskOptions::RAM;
    info.SetOptions(options);
    EXPECT_EQ(info.GetOptions(), options);
}

