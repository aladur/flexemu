#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ffilecnt.h"
#include "ffilebuf.h"
#include "filfschk.h"
#include "typedefs.h"
#include <array>
#include <memory>
#include <filesystem>


using ::testing::StartsWith;
namespace fs = std::filesystem;

class test_filfschk : public ::testing::Test
{
protected:
    const std::string diskPath{"/tmp/disk.dsk"};
    const std::string subdir{"data/"};
    const std::string fileNameOnDisk{"cat.cmd"};
    std::unique_ptr<FlexDisk> disk;

    void SetUp() override
    {
        disk.reset(FlexDisk::Create(diskPath, FileTimeAccess::NONE, 35, 10));
        ASSERT_NE(disk.get(), nullptr);
        FlexFileBuffer buffer;
        ASSERT_TRUE(buffer.ReadFromFile(subdir + fileNameOnDisk,
                    FileTimeAccess::NONE));
        ASSERT_TRUE(disk->WriteFromBuffer(buffer));
    }

    void TearDown() override
    {
        fs::remove(diskPath);
    }

    using dirfct_t = void (s_dir_sector &);
    using sisfct_t = void (s_sys_info_sector &);
    using sectorfct_t = void (std::array<Byte, SECTOR_SIZE> &);

    void ModifyDirSector(dirfct_t fct)
    {
        s_dir_sector dirSector{};
        auto *byteBuffer = reinterpret_cast<Byte *>(&dirSector);

        ASSERT_TRUE(disk->ReadSector(byteBuffer, 0, 5));
        fct(dirSector);
        ASSERT_TRUE(disk->WriteSector(byteBuffer, 0, 5));
    }

    void ModifySysInfoSector(sisfct_t fct)
    {
        s_sys_info_sector sis{};
        auto *byteBuffer = reinterpret_cast<Byte *>(&sis);

        ASSERT_TRUE(disk->ReadSector(byteBuffer, 0, 3));
        fct(sis);
        ASSERT_TRUE(disk->WriteSector(byteBuffer, 0, 3));
    }

    void ModifySector(int track, int sector, sectorfct_t fct)
    {
        std::array<Byte, SECTOR_SIZE> sectorBuffer{};
        auto *byteBuffer = reinterpret_cast<Byte *>(&sectorBuffer);

        ASSERT_TRUE(disk->ReadSector(byteBuffer, track, sector));
        fct(sectorBuffer);
        ASSERT_TRUE(disk->WriteSector(byteBuffer, track, sector));
    }

    static void PrintResult(const FlexDiskCheckResultItems &items)
    {
        const auto *testname =
            ::testing::UnitTest::GetInstance()->current_test_info()->name();
        std::cout << "Results of " << testname << "\n";
        for (const auto &item : items)
        {
            std::cout << "  " << item << "\n";
        }
    }

    template<class T>
    T GetFirstItemOfType(const FlexDiskCheckResultItems &items)
    {
        for (const auto &item : items)
        {
            T result = dynamic_cast<T>(item.get());
            if (result != nullptr)
            {
                return result;
            }
        }

        return nullptr;
    }

};

TEST_F(test_filfschk, check_ValidDisk)
{
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::NONE);
    EXPECT_TRUE(checker.CheckFileSystem());
    EXPECT_TRUE(checker.IsValid());
    const auto &items = checker.GetResult();
    EXPECT_TRUE(items.empty());
}

TEST_F(test_filfschk, check_NullFile)
{
    ModifyDirSector([](s_dir_sector &dirSector){
        auto *dirEntry = &dirSector.dir_entries[1];
        // Manipulate directory by adding a null file entry without 00-00
        // start/end track/sector.
        dirEntry->filename[0] = 'F';
        dirEntry->file_ext[0] = 'E';
        dirEntry->month = 11;
        dirEntry->day = 5;
        dirEntry->year = 80;
    });
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::NONE);
    EXPECT_FALSE(checker.CheckFileSystem());
    EXPECT_FALSE(checker.IsValid());
    const auto &items = checker.GetResult();
    EXPECT_EQ(items.size(), 1U);
    const auto *item = GetFirstItemOfType<NullFile *>(items);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->type, FlexDiskCheckResultItem::Type::Info);
    EXPECT_EQ(item->name, "f.e");
}

TEST_F(test_filfschk, check_BadStart1)
{
    ModifyDirSector([](s_dir_sector &dirSector){
        auto *dirEntry = &dirSector.dir_entries[1];
        // Manipulate directory by adding a second directory entry which
        // end link points to a sector of the first directory entry.
        // Set a valid date to avoid more findings.
        dirEntry->start = st_t{0, 1};
        dirEntry->end = st_t{1, 3};
        dirEntry->filename[0] = 'F';
        dirEntry->file_ext[0] = 'E';
        dirEntry->month = 11;
        dirEntry->day = 5;
        dirEntry->year = 80;
    });
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::NONE);
    EXPECT_FALSE(checker.CheckFileSystem());
    const auto &items = checker.GetResult();
    EXPECT_EQ(items.size(), 1U);
    const auto *item = GetFirstItemOfType<BadStart *>(items);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->type, FlexDiskCheckResultItem::Type::Error);
    const st_t start{0, 1};
    EXPECT_EQ(item->start, start);
    EXPECT_EQ(item->name, "f.e");
}

TEST_F(test_filfschk, check_BadStart2)
{
    ModifyDirSector([](s_dir_sector &dirSector){
        auto *dirEntry = &dirSector.dir_entries[0];
        // Manipulate start link of first file to 00-00.
        dirEntry->start = st_t{};
    });
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::NONE);
    EXPECT_FALSE(checker.CheckFileSystem());
    const auto &items = checker.GetResult();
    EXPECT_EQ(items.size(), 2U); // OK
    const auto *item = GetFirstItemOfType<BadStart *>(items);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->type, FlexDiskCheckResultItem::Type::Error);
    const st_t start{};
    EXPECT_EQ(item->start, start);
    EXPECT_EQ(item->name, fileNameOnDisk);
}

TEST_F(test_filfschk, check_BadEnd1)
{
    ModifyDirSector([](s_dir_sector &dirSector){
        auto *dirEntry = &dirSector.dir_entries[1];
        // Manipulate directory to add a second file which end link points to
        // an invalid sector. Set a valid date to avoid more findings.
        dirEntry->start = st_t{1, 1};
        dirEntry->end = st_t{0, 1};
        dirEntry->filename[0] = 'F';
        dirEntry->file_ext[0] = 'E';
        dirEntry->month = 11;
        dirEntry->day = 5;
        dirEntry->year = 80;
    });
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::NONE);
    EXPECT_FALSE(checker.CheckFileSystem());
    const auto &items = checker.GetResult();
    EXPECT_EQ(items.size(), 1U);
    const auto *item = GetFirstItemOfType<BadEnd *>(items);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->type, FlexDiskCheckResultItem::Type::Error);
    const st_t end{0, 1};
    EXPECT_EQ(item->end, end);
    EXPECT_EQ(item->name, "f.e");
}

TEST_F(test_filfschk, check_MultipleLinkInputs)
{
    ModifySector(0x22, 9, [](std::array<Byte, SECTOR_SIZE> &sectorBuffer){
        // Manipulate sector 22-09 to have a link to sector 01-02.
        sectorBuffer[0] = 1;
        sectorBuffer[1] = 2;
    });
    ModifySysInfoSector([](s_sys_info_sector &sis){
        // Update free sector count.
        sis.sir.free[0] = 1;
        sis.sir.free[1] = 82;
    });
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::NONE);
    EXPECT_FALSE(checker.CheckFileSystem());
    const auto &items = checker.GetResult();
    EXPECT_EQ(items.size(), 2U); // OK
    const auto *item = GetFirstItemOfType<MultipleLinkInputs *>(items);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->type, FlexDiskCheckResultItem::Type::Error);
    const st_t current{1, 2};
    EXPECT_EQ(item->current, current);
    EXPECT_EQ(item->name, "Free Chain");
    EXPECT_EQ(item->inputs.size(), 2U);
}

TEST_F(test_filfschk, check_LinkAndFileInput)
{
    ModifySector(0x22, 9, [](std::array<Byte, SECTOR_SIZE> &sectorBuffer){
        // Manipulate sector 22-09 to have a link to sector 01-01 (which is
        // the start sector of first file).
        sectorBuffer[0] = 1;
        sectorBuffer[1] = 1;
    });
    ModifySysInfoSector([](s_sys_info_sector &sis){
        // Manipulate free sector count.
        sis.sir.free[0] = 1;
        sis.sir.free[1] = 83;
    });
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::NONE);
    EXPECT_FALSE(checker.CheckFileSystem());
    const auto &items = checker.GetResult();
    EXPECT_EQ(items.size(), 2U); // OK
    const auto *item = GetFirstItemOfType<LinkAndFileInput *>(items);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->type, FlexDiskCheckResultItem::Type::Error);
    const st_t current{1, 1};
    EXPECT_EQ(item->current, current);
    EXPECT_EQ(item->name, fileNameOnDisk);
    const st_t input{0x22, 9};
    EXPECT_EQ(item->input, input);
}

TEST_F(test_filfschk, check_LinkAfterEnd)
{
    ModifySector(1, 3, [](std::array<Byte, SECTOR_SIZE> &sectorBuffer){
        // Manipulate sector 01-03 (which is the last sector of first file)
        // to have a link into the free chain.
        sectorBuffer[0] = 1;
        sectorBuffer[1] = 4;
    });
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::NONE);
    EXPECT_FALSE(checker.CheckFileSystem());
    const auto &items = checker.GetResult();
    EXPECT_EQ(items.size(), 1U);
    const auto *item = GetFirstItemOfType<LinkAfterEnd *>(items);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->type, FlexDiskCheckResultItem::Type::Error);
    const st_t end{1, 3};
    EXPECT_EQ(item->end, end);
    EXPECT_EQ(item->name, fileNameOnDisk);
}

TEST_F(test_filfschk, check_InconsistentRecordSize)
{
    ModifyDirSector([](s_dir_sector &dirSector){
        auto *dirEntry = &dirSector.dir_entries[0];
        // Manipulate file size.
        dirEntry->records[1] = 4;
    });
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::NONE);
    EXPECT_FALSE(checker.CheckFileSystem());
    const auto &items = checker.GetResult();
    EXPECT_EQ(items.size(), 1U);
    const auto *item = GetFirstItemOfType<InconsistentRecordSize *>(items);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->type, FlexDiskCheckResultItem::Type::Warning);
    EXPECT_EQ(item->records, 4);
    EXPECT_EQ(item->sectors, 3);
    EXPECT_EQ(item->name, fileNameOnDisk);
}

TEST_F(test_filfschk, check_DiscontiguousRecordNr)
{
    ModifySector(1, 3, [](std::array<Byte, SECTOR_SIZE> &sectorBuffer){
        // Manipulate sector 01-03 (which is the last sector of first file)
        // to have record number 4 (instead of 3).
        sectorBuffer[3] = 4;
    });
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::NONE);
    EXPECT_FALSE(checker.CheckFileSystem());
    const auto &items = checker.GetResult();
    EXPECT_EQ(items.size(), 1U);
    const auto *item = GetFirstItemOfType<DiscontiguousRecordNr *>(items);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->type, FlexDiskCheckResultItem::Type::Warning);
    st_t current{1, 3};
    EXPECT_EQ(item->current, current);
    EXPECT_EQ(item->record_nr, 4);
    EXPECT_EQ(item->expected_record_nr, 3);
    EXPECT_EQ(item->name, fileNameOnDisk);
}

TEST_F(test_filfschk, check_LostSectors)
{
    ModifySector(0x22, 1, [](std::array<Byte, SECTOR_SIZE> &sectorBuffer){
        // Manipulate sector 22-01 to be last sector of free chain.
        sectorBuffer[0] = 0;
        sectorBuffer[1] = 0;
    });
    ModifySysInfoSector([](s_sys_info_sector &sis){
        // Manipulate end of free chain and free sector count to avoid more
        // findings.
        sis.sir.fc_end = st_t{0x22, 1};
        sis.sir.free[0] = 1;
        sis.sir.free[1] = 72;
    });
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::NONE);
    EXPECT_FALSE(checker.CheckFileSystem());
    const auto &items = checker.GetResult();
    EXPECT_EQ(items.size(), 1U);
    const auto *item = GetFirstItemOfType<LostSectors *>(items);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->type, FlexDiskCheckResultItem::Type::Info);
    st_t start{0x22, 2};
    st_t end{0x22, 0x0A};
    EXPECT_EQ(item->start, start);
    EXPECT_EQ(item->end, end);
    EXPECT_EQ(item->sectors, 9);
    EXPECT_THAT(item->name, StartsWith("Lost"));
}

TEST_F(test_filfschk, check_HasCycles)
{
    ModifySector(1, 2, [](std::array<Byte, SECTOR_SIZE> &sectorBuffer){
        // Manipulate sector 01-02 (the second sector of first file) to have
        // a link back to 01-01.
        sectorBuffer[0] = 1;
        sectorBuffer[1] = 1;
    });
    ModifyDirSector([](s_dir_sector &dirSector){
        auto *dirEntry = &dirSector.dir_entries[0];
        // Manipulate file size to avoid more findings.
        dirEntry->records[1] = 2;
    });
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::NONE);
    EXPECT_FALSE(checker.CheckFileSystem());
    const auto &items = checker.GetResult();
    EXPECT_EQ(items.size(), 3U); // OK
    const auto *item = GetFirstItemOfType<HasCycle *>(items);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->type, FlexDiskCheckResultItem::Type::Error);
    st_t from{1, 2};
    st_t back_to{1, 1};
    EXPECT_EQ(item->from, from);
    EXPECT_EQ(item->back_to, back_to);
    EXPECT_EQ(item->name, fileNameOnDisk);
}

TEST_F(test_filfschk, check_BadLink)
{
    ModifySector(0x22, 9, [](std::array<Byte, SECTOR_SIZE> &sectorBuffer){
        // Manipulate sector 22-09 by creating a bad link to 30-01.
        sectorBuffer[0] = 48;
        sectorBuffer[1] = 1;
    });
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::NONE);
    EXPECT_FALSE(checker.CheckFileSystem());
    const auto &items = checker.GetResult();
    EXPECT_EQ(items.size(), 2U); // OK
    const auto *item = GetFirstItemOfType<BadLink *>(items);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->type, FlexDiskCheckResultItem::Type::Warning);
    st_t bad{0x30, 1};
    st_t current{0x22, 9};
    EXPECT_EQ(item->bad, bad);
    EXPECT_EQ(item->current, current);
    EXPECT_EQ(item->name, "Free Chain");
}

TEST_F(test_filfschk, check_BadFileDate)
{
    ModifyDirSector([](s_dir_sector &dirSector){
        auto *dirEntry = &dirSector.dir_entries[0];
        // Manipulate directory by setting a bad date for the first file.
        dirEntry->month = 13;
        dirEntry->day = 5;
        dirEntry->year = 80;
    });
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::NONE);
    EXPECT_FALSE(checker.CheckFileSystem());
    const auto &items = checker.GetResult();
    EXPECT_EQ(items.size(), 1U);
    const auto *item = GetFirstItemOfType<BadDate *>(items);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->type, FlexDiskCheckResultItem::Type::Info);
    EXPECT_EQ(item->day, 5);
    EXPECT_EQ(item->month, 13);
    EXPECT_EQ(item->year, 80);
    EXPECT_EQ(item->name, fileNameOnDisk);
}

TEST_F(test_filfschk, check_BadDiskDate)
{
    ModifySysInfoSector([](s_sys_info_sector &sis){
        // Update free sector count.
        sis.sir.month = 13;
        sis.sir.day = 5;
        sis.sir.year = 80;
    });
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::Set);
    EXPECT_FALSE(checker.CheckFileSystem());
    const auto &items = checker.GetResult();
    EXPECT_EQ(items.size(), 1U);
    const auto *item = GetFirstItemOfType<BadDate *>(items);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->type, FlexDiskCheckResultItem::Type::Info);
    EXPECT_EQ(item->day, 5);
    EXPECT_EQ(item->month, 13);
    EXPECT_EQ(item->year, 80);
    EXPECT_EQ(item->name, "The disk");
}

TEST_F(test_filfschk, check_BadFileTime)
{
    ModifyDirSector([](s_dir_sector &dirSector){
        auto *dirEntry = &dirSector.dir_entries[0];
        // Manipulate directory by setting a bad time for the first file.
        dirEntry->hour = 14;
        dirEntry->minute = 60;
    });
    auto checker = FlexDiskCheck(*disk, FileTimeAccess::Set);
    EXPECT_FALSE(checker.CheckFileSystem());
    const auto &items = checker.GetResult();
    EXPECT_EQ(items.size(), 1U);
    const auto *item = GetFirstItemOfType<BadTime *>(items);
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->type, FlexDiskCheckResultItem::Type::Info);
    EXPECT_EQ(item->hour, 14);
    EXPECT_EQ(item->minute, 60);
    EXPECT_EQ(item->name, fileNameOnDisk);
}

