// Test IFlexDiskByFile interface.
//
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fdirent.h"
#include "ffilecnt.h"
#include "rfilecnt.h"
#include "dircont.h"
#include "ifilecnt.h"
#include "flexerr.h"
#include "fcopyman.h"
#include "ffilebuf.h"
#include "fixt_filecont.h"
#include <array>
#include <memory>
#include <fstream>
#include <filesystem>
#include <numeric>
#include <fmt/format.h>


using ::testing::Throws;
using ::testing::StartsWith;
using ::testing::EndsWith;

namespace fs = std::filesystem;
using FlexDiskByFilePtr = std::unique_ptr<IFlexDiskByFile>;

class test_IFlexDiskByFile : public test_FlexDiskFixture
{
protected:
    std::array<std::array<FlexDiskByFilePtr, 3>, 6> disks;

    void SetUp() override
    {
        test_FlexDiskFixture::SetUp();

        const auto mode = std::ios::in | std::ios::out | std::ios::binary;
        const auto romode = std::ios::in | std::ios::binary;

        for (int idx = RO; idx <= TGT; ++idx)
        {
            const auto ios_mode = (idx == RO || idx == ROM) ? romode : mode;
            const auto &ft = (idx == FT) ? with_ft : no_ft;
            FlexDisk *pdisk{};

            for (int tidx = DSK; tidx <= FLX; ++tidx)
            {
                pdisk = (idx == RAM || idx == ROM) ?
                    new FlexRamDisk(diskPaths[idx][tidx], ios_mode, ft) :
                    new FlexDisk(diskPaths[idx][tidx], ios_mode, ft);
                disks[idx][tidx].reset(cast(pdisk));
                ASSERT_NE(disks[idx][tidx].get(), nullptr);
            }
        }

        std::vector<int> dirIndices{RO, RW, FT, TGT};
        for (int idx : dirIndices)
        {
            const auto &ft = (idx == FT) ? with_ft : no_ft;
            auto *pdir =
                    new FlexDirectoryDiskByFile(diskPaths[idx][DIR], ft);
            disks[idx][DIR].reset(cast(pdir));
            ASSERT_NE(disks[idx][DIR].get(), nullptr);
        }
    }

    void TearDown() override
    {
        for (int idx = RO; idx <= TGT; ++idx)
        {
            disks[idx][DSK].reset();
            disks[idx][FLX].reset();
            disks[idx][DIR].reset();
        }

        test_FlexDiskFixture::TearDown();
    }

    static IFlexDiskByFile *cast(FlexDisk *disk)
    {
         return static_cast<IFlexDiskByFile *>(disk);
    }

    static IFlexDiskByFile *cast(FlexDirectoryDiskByFile *dirDisk)
    {
         return static_cast<IFlexDiskByFile *>(dirDisk);
    }
};

TEST_F(test_IFlexDiskByFile, fct_FindFile)
{
    const std::vector<int> indices{RO, ROM, RW, RAM};
    for (int idx : indices)
    {
        for (auto &disk : disks[idx])
        {
            FlexDirEntry dirEntry;

            if (!disk)
            {
                continue;
            }
            std::string wildcard = "TEST01.TXT";
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            EXPECT_FALSE(dirEntry.IsEmpty());
            EXPECT_EQ(dirEntry.GetTotalFileName(), wildcard);
            EXPECT_EQ(dirEntry.GetFileSize(), SECTOR_SIZE);
            EXPECT_EQ(dirEntry.GetAttributes(), FLX_READONLY);
            wildcard = "TEST10.TXT";
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            EXPECT_FALSE(dirEntry.IsEmpty());
            EXPECT_EQ(dirEntry.GetTotalFileName(), wildcard);
            EXPECT_EQ(dirEntry.GetFileSize(), SECTOR_SIZE * 10);
            EXPECT_EQ(dirEntry.GetAttributes(), 0);
            wildcard = "TEST02.BIN";
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            EXPECT_FALSE(dirEntry.IsEmpty());
            EXPECT_EQ(dirEntry.GetTotalFileName(), wildcard);
            EXPECT_EQ(dirEntry.GetFileSize(), SECTOR_SIZE * 2);
            EXPECT_EQ(dirEntry.GetAttributes(), 0);
            wildcard = "TEST09.BIN";
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            EXPECT_FALSE(dirEntry.IsEmpty());
            EXPECT_EQ(dirEntry.GetTotalFileName(), wildcard);
            EXPECT_EQ(dirEntry.GetFileSize(), SECTOR_SIZE * 9);
            EXPECT_EQ(dirEntry.GetAttributes(), 0);
            wildcard = "test05.bin";
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            EXPECT_FALSE(dirEntry.IsEmpty());
            EXPECT_EQ(dirEntry.GetTotalFileName(), flx::toupper(wildcard));
            EXPECT_EQ(dirEntry.GetFileSize(), SECTOR_SIZE * 5);
            EXPECT_EQ(dirEntry.GetAttributes(), 0);
            wildcard = "";
            EXPECT_FALSE(disk->FindFile(wildcard, dirEntry));
            EXPECT_TRUE(dirEntry.IsEmpty());
            wildcard = "TEST01.BI";
            EXPECT_FALSE(disk->FindFile(wildcard, dirEntry));
            EXPECT_TRUE(dirEntry.IsEmpty());
            wildcard = "TEST*.BIN";
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            EXPECT_FALSE(dirEntry.IsEmpty());
            EXPECT_THAT(dirEntry.GetTotalFileName(), StartsWith("TEST"));
            EXPECT_THAT(dirEntry.GetTotalFileName(), EndsWith("BIN"));
            wildcard = "TEST?0.BIN";
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            EXPECT_FALSE(dirEntry.IsEmpty());
            EXPECT_THAT(dirEntry.GetTotalFileName(), StartsWith("TEST"));
            EXPECT_THAT(dirEntry.GetTotalFileName(), EndsWith("BIN"));
            wildcard = "*.*";
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            EXPECT_FALSE(dirEntry.IsEmpty());
            EXPECT_THAT(dirEntry.GetTotalFileName(), StartsWith("TEST"));
            wildcard = "*";
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            EXPECT_FALSE(dirEntry.IsEmpty());
            EXPECT_THAT(dirEntry.GetTotalFileName(), StartsWith("TEST"));
        }
    }
}

TEST_F(test_IFlexDiskByFile, fct_DeleteFile)
{
    const std::vector<int> indices{RW, RAM};
    for (int idx : indices)
    {
        for (auto &disk : disks[idx])
        {
            FlexDirEntry dirEntry;

            if (!disk)
            {
                continue;
            }

            std::string wildcard = "TEST02.TXT";
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            EXPECT_TRUE(disk->DeleteFile(wildcard));
            EXPECT_FALSE(disk->FindFile(wildcard, dirEntry));
            EXPECT_TRUE(disk->FindFile("TEST*.TXT", dirEntry));
            wildcard = "TEST?0.*";
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            EXPECT_TRUE(disk->DeleteFile(wildcard));
            EXPECT_FALSE(disk->FindFile(wildcard, dirEntry));
            EXPECT_TRUE(disk->FindFile("TEST*.*", dirEntry));
            wildcard = "*";
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            EXPECT_TRUE(disk->DeleteFile(wildcard));
            EXPECT_FALSE(disk->FindFile(wildcard, dirEntry));
            EXPECT_FALSE(disk->FindFile("TEST*.*", dirEntry));
        }
    }
}

TEST_F(test_IFlexDiskByFile, fct_RenameFile)
{
    const std::vector<int> indices{RW, RAM};
    for (int idx : indices)
    {
        for (auto &disk : disks[idx])
        {
            FlexDirEntry dirEntry;

            if (!disk)
            {
                continue;
            }

            std::string filename = "TEST01.TXT";
            EXPECT_TRUE(disk->FindFile(filename, dirEntry));
            EXPECT_TRUE(disk->RenameFile(filename, "TEST99.TXT"));
            EXPECT_FALSE(disk->FindFile(filename, dirEntry));
            EXPECT_TRUE(disk->FindFile("TEST99.TXT", dirEntry));
            filename = "test10.txt";
            std::string newname = "test90.txt";
            EXPECT_TRUE(disk->FindFile(filename, dirEntry));
            EXPECT_TRUE(disk->RenameFile(filename, newname));
            EXPECT_FALSE(disk->FindFile(filename, dirEntry));
            EXPECT_TRUE(disk->FindFile(newname, dirEntry));
            filename = "TEST05.BIN";
            EXPECT_FALSE(disk->RenameFile(filename, filename));
            EXPECT_TRUE(disk->FindFile(filename, dirEntry));
            newname = flx::toupper(newname);
            EXPECT_THAT([&](){ disk->RenameFile("TEST02.TXT", newname); },
                    testing::Throws<FlexException>());
            newname = "TEST999.TXT";
            EXPECT_THAT([&](){ disk->RenameFile("NOTEXIST.TXT", newname); },
                    testing::Throws<FlexException>());
            EXPECT_THAT([&](){ disk->RenameFile("TEST*.BIN", newname); },
                    testing::Throws<FlexException>());
            EXPECT_THAT([&](){ disk->RenameFile("TEST?.BIN", newname); },
                    testing::Throws<FlexException>());
            EXPECT_THAT([&](){ disk->RenameFile("TEST[012]0.BIN", newname); },
                    testing::Throws<FlexException>());
            filename = "TEST01.BIN";
            EXPECT_THAT([&](){ disk->RenameFile(filename, "TEST*.BIN"); },
                    testing::Throws<FlexException>());
            EXPECT_THAT([&](){ disk->RenameFile(filename, "TEST?.BIN"); },
                    testing::Throws<FlexException>());
            EXPECT_THAT([&](){ disk->RenameFile(filename, "TEST[012]0.BIN"); },
                    testing::Throws<FlexException>());
        }
    }
}

TEST_F(test_IFlexDiskByFile, fct_SetAttributes)
{
    const std::vector<int> indices{RW, RAM};
    for (int idx : indices)
    {
        for (auto &disk : disks[idx])
        {
            FlexDirEntry dirEntry;

            if (!disk)
            {
                continue;
            }

            std::string filename = "TEST02.TXT";
            EXPECT_TRUE(disk->FindFile(filename, dirEntry));
            EXPECT_EQ(dirEntry.GetAttributes(), 0);
            EXPECT_TRUE(disk->SetAttributes(filename, FLX_READONLY, 0));
            EXPECT_TRUE(disk->FindFile(filename, dirEntry));
            EXPECT_EQ(dirEntry.GetAttributes(), FLX_READONLY);
            EXPECT_TRUE(disk->SetAttributes(filename, 0, FLX_READONLY));
            EXPECT_TRUE(disk->FindFile(filename, dirEntry));
            EXPECT_EQ(dirEntry.GetAttributes(), 0);

            filename = "*.BIN";
            const std::vector<const char *> filenames{
                "TEST01.BIN", "TEST02.BIN", "TEST03.BIN", "TEST04.BIN",
                "TEST05.BIN", "TEST06.BIN", "TEST07.BIN", "TEST08.BIN",
                "TEST09.BIN", "TEST10.BIN"
            };
            for (const auto &testfile : filenames)
            {
                EXPECT_TRUE(disk->FindFile(testfile, dirEntry));
                EXPECT_EQ(dirEntry.GetAttributes(), 0);
            }
            EXPECT_TRUE(disk->SetAttributes(filename, FLX_READONLY, 0));
            for (const auto &testfile : filenames)
            {
                EXPECT_TRUE(disk->FindFile(testfile, dirEntry));
                EXPECT_EQ(dirEntry.GetAttributes(), FLX_READONLY);
            }
            EXPECT_TRUE(disk->SetAttributes(filename, 0, FLX_READONLY));
            for (const auto &testfile : filenames)
            {
                EXPECT_TRUE(disk->FindFile(testfile, dirEntry));
                EXPECT_EQ(dirEntry.GetAttributes(), 0);
            }

            if (disk->GetPath() == diskPaths[RW][DSK])
            {
                // FLX_NODELETE, FLX_NOREAD and FLX_NOCAT is only supported
                // with FlexDiskByFile (but not FlexDirectoryByFile).
                EXPECT_TRUE(disk->SetAttributes(filename,
                            FLX_NODELETE | FLX_NOCAT, FLX_READONLY));
                EXPECT_TRUE(disk->FindFile(filename, dirEntry));
                EXPECT_EQ(dirEntry.GetAttributes(), FLX_NODELETE | FLX_NOCAT);
                EXPECT_TRUE(disk->SetAttributes(filename, FLX_NOREAD,
                            FLX_NODELETE | FLX_NOCAT));
                EXPECT_TRUE(disk->FindFile(filename, dirEntry));
                EXPECT_EQ(dirEntry.GetAttributes(), FLX_NOREAD);
            }
        }
    }
}

TEST_F(test_IFlexDiskByFile, fct_ReadToBuffer)
{
    const std::vector<int> indices{RO, ROM, RW, RAM};
    for (int idx : indices)
    {
        for (auto &disk : disks[idx])
        {
            if (!disk)
            {
                continue;
            }

            std::string filename = "TEST05.TXT";
            auto buffer = disk->ReadToBuffer(filename);
            EXPECT_FALSE(buffer.IsEmpty());
            EXPECT_EQ(buffer.GetFileSize(), (SECTOR_SIZE - 4) * 5);
            EXPECT_EQ(buffer.GetFilename(), filename);
            EXPECT_TRUE(buffer.IsFlexTextFile());
            EXPECT_EQ(buffer.GetDate(), BDate(11, 8, 2024));
            EXPECT_EQ(buffer.GetTime(), BTime());

            filename = "TEST10.BIN";
            buffer = disk->ReadToBuffer(filename);
            EXPECT_FALSE(buffer.IsEmpty());
            EXPECT_EQ(buffer.GetFileSize(), (SECTOR_SIZE - 4) * 10);
            EXPECT_EQ(buffer.GetFilename(), filename);
            EXPECT_FALSE(buffer.IsFlexTextFile());
            EXPECT_EQ(buffer.GetDate(), BDate(11, 8, 2024));
            EXPECT_EQ(buffer.GetTime(), BTime());

            EXPECT_THAT([&](){ disk->ReadToBuffer("NOTEXIST.TXT"); },
                    testing::Throws<FlexException>());
            EXPECT_THAT([&](){ disk->ReadToBuffer("TEST*.BIN"); },
                    testing::Throws<FlexException>());
            EXPECT_THAT([&](){ disk->ReadToBuffer("TEST?.BIN"); },
                    testing::Throws<FlexException>());
            EXPECT_THAT([&](){ disk->ReadToBuffer("TEST[012]0.BIN"); },
                    testing::Throws<FlexException>());
        }
    }

    for (auto &disk : disks[FT])
    {
        auto buffer = disk->ReadToBuffer("TEST05.BIN");
        EXPECT_EQ(buffer.GetDate(), BDate(11, 8, 2024));
        EXPECT_EQ(buffer.GetTime(), BTime(22, 1));

        buffer = disk->ReadToBuffer("TEST10.BIN");
        EXPECT_EQ(buffer.GetDate(), BDate(11, 8, 2024));
        EXPECT_EQ(buffer.GetTime(), BTime(22, 1));
    }

    for (auto &disk : disks[RW])
    {
        auto buffer = disk->ReadToBuffer("TEST05.BIN");
        EXPECT_EQ(buffer.GetDate(), BDate(11, 8, 2024));
        EXPECT_EQ(buffer.GetTime(), BTime{});

        buffer = disk->ReadToBuffer("TEST10.BIN");
        EXPECT_EQ(buffer.GetDate(), BDate(11, 8, 2024));
        EXPECT_EQ(buffer.GetTime(), BTime{});
    }
}

TEST_F(test_IFlexDiskByFile, fct_WriteFromBuffer)
{
    std::string filename{"test.txt"};
    auto ft = FileTimeAccess::Get | FileTimeAccess::Set;
    auto path = createFile("/tmp", filename, true, 22);
    setDateTime(path, BDate(11, 8, 2024), BTime(22, 1), ft);

    for (auto &disk : disks[FT])
    {
        FlexFileBuffer buffer;
        FlexDirEntry dirEntry;

        ASSERT_TRUE(buffer.ReadFromFile(path, ft));
        ASSERT_TRUE(disk->WriteFromBuffer(buffer));
        ASSERT_TRUE(disk->FindFile(filename, dirEntry));
        EXPECT_FALSE(dirEntry.IsEmpty());
        filename = flx::toupper(filename);
        EXPECT_EQ(dirEntry.GetTotalFileName(), filename);
        EXPECT_EQ(dirEntry.GetFileSize(), SECTOR_SIZE * 22);
        EXPECT_EQ(dirEntry.GetDate(), BDate(11, 8, 2024));
        EXPECT_EQ(dirEntry.GetTime(), BTime(22, 1));
    }

    const std::vector<int> indices{RW, RAM};
    for (int idx : indices)
    {
        for (auto &disk : disks[idx])
        {
            FlexFileBuffer buffer;
            FlexDirEntry dirEntry;

            if (!disk)
            {
                continue;
            }

            ASSERT_TRUE(buffer.ReadFromFile(path, ft));
            ASSERT_TRUE(disk->WriteFromBuffer(buffer));
            ASSERT_TRUE(disk->FindFile(filename, dirEntry));
            EXPECT_FALSE(dirEntry.IsEmpty());
            filename = flx::toupper(filename);
            EXPECT_EQ(dirEntry.GetTotalFileName(), filename);
            EXPECT_EQ(dirEntry.GetFileSize(), SECTOR_SIZE * 22);
            EXPECT_EQ(dirEntry.GetDate(), BDate(11, 8, 2024));
            EXPECT_EQ(dirEntry.GetTime(), BTime());
        }
    }

    const std::vector<int> roindices{RO, ROM};
    for (int idx : roindices)
    {
        for (auto &disk : disks[idx])
        {
            FlexFileBuffer buffer;
            FlexDirEntry dirEntry;

            if (!disk)
            {
                continue;
            }

            ASSERT_TRUE(buffer.ReadFromFile(path, ft));
            EXPECT_THAT([&](){ disk->WriteFromBuffer(buffer); },
                    testing::Throws<FlexException>());
            ASSERT_FALSE(disk->FindFile(filename, dirEntry));
            EXPECT_TRUE(dirEntry.IsEmpty());
        }
    }

    fs::remove(path);
}

TEST_F(test_IFlexDiskByFile, fct_FileCopy)
{
    FlexCopyManager::autoTextConversion = true;

    const std::vector<int> indices{RO, ROM, RW, RAM};
    for (int idx : indices)
    {
        for (auto &disk : disks[idx])
        {
            if (!disk)
            {
                continue;
            }

            for (auto &tgt : disks[TGT])
            {
                std::string src = "TEST10.BIN";
                std::string dest = "NEW10.BIN";
                tgt->DeleteFile(dest);
                EXPECT_FALSE(disk->FileCopy(src, dest, *tgt.get()));
                EXPECT_THAT([&](){ disk->FileCopy(src, dest, *tgt.get()); },
                        testing::Throws<FlexException>());
                src = "TEST10.TXT";
                dest = "NEW10.TXT";
                tgt->DeleteFile(dest);
                // Result true means that a text file conversion was executed.
                auto result = disk->FileCopy(src, dest, *tgt.get());
                const auto compareTypes = TYPE_DISKFILE | TYPE_DIRECTORY;
                auto srcType = disk->GetFlexDiskType() & compareTypes;
                auto tgtType = tgt->GetFlexDiskType() & compareTypes;
                EXPECT_EQ(result, srcType != tgtType);
                src = "NOTEXIST.BIN";
                dest = "NEW99.BIN";
                EXPECT_THAT([&](){ disk->FileCopy(src, dest, *tgt.get()); },
                       testing::Throws<FlexException>());
                src = "TEST*.BIN";
                EXPECT_THAT([&](){ disk->FileCopy(src, dest, *tgt.get()); },
                       testing::Throws<FlexException>());
                src = "TEST?.BIN";
                EXPECT_THAT([&](){ disk->FileCopy(src, dest, *tgt.get()); },
                       testing::Throws<FlexException>());
                src = "TEST[012]0.BIN";
                EXPECT_THAT([&](){ disk->FileCopy(src, dest, *tgt.get()); },
                       testing::Throws<FlexException>());
                src = "TEST10.BIN";
                dest = "TEST*.BIN";
                EXPECT_THAT([&](){ disk->FileCopy(src, dest, *tgt.get()); },
                       testing::Throws<FlexException>());
                dest = "TEST?.BIN";
                EXPECT_THAT([&](){ disk->FileCopy(src, dest, *tgt.get()); },
                       testing::Throws<FlexException>());
                dest = "TEST[012]0.BIN";
                EXPECT_THAT([&](){ disk->FileCopy(src, dest, *tgt.get()); },
                       testing::Throws<FlexException>());
                EXPECT_THAT([&](){ disk->FileCopy(src, dest, *tgt.get()); },
                       testing::Throws<FlexException>());
                EXPECT_THAT([&](){ disk->FileCopy(src, dest, *disk.get()); },
                        testing::Throws<FlexException>());
            }
        }
    }
}

TEST_F(test_IFlexDiskByFile, fct_GetSupportedAttributes)
{
    for (auto &disk : disks[RW])
    {
        const std::string attr = (disk->GetPath() == diskPaths[RW][DIR]) ?
            "W" : "WDRC";
        EXPECT_EQ(disk->GetSupportedAttributes(), attr);
    }
}

TEST_F(test_IFlexDiskByFile, fcts_begin_end)
{
    const std::vector<int> indices{RO, ROM, RW, RAM, FT};
    for (int idx : indices)
    {
        for (auto &disk : disks[idx])
        {
            if (!disk)
            {
                continue;
            }

            FlexDiskIterator iter1;
            int count = 0;
            for (iter1 = disk->begin(); iter1 != disk->end(); ++iter1)
            {
                ++count;
            }
            EXPECT_EQ(count, 20);

            FlexDiskIterator iter2("*.BIN");
            count = 0;
            for (iter2 = disk->begin(); iter2 != disk->end(); ++iter2)
            {
                ++count;
            }
            EXPECT_EQ(count, 10);

            FlexDiskIterator iter3("TEST?0.???");
            count = 0;
            for (iter3 = disk->begin(); iter3 != disk->end(); ++iter3)
            {
                ++count;
            }
            EXPECT_EQ(count, 2);

            FlexDiskIterator iter4("NOTEXIST.???");
            count = 0;
            for (iter4 = disk->begin(); iter4 != disk->end(); ++iter4)
            {
                ++count;
            }
            EXPECT_EQ(count, 0);
        }
    }

    for (auto &disk : disks[TGT])
    {
        FlexDiskIterator iter;

        iter = disk->begin();
        EXPECT_TRUE(iter == disk->end());
    }
}

TEST_F(test_IFlexDiskByFile, fcts_ReadOnly)
{
    const std::vector<int> indices{RO, ROM};
    for (int idx : indices)
    {
        for (auto &disk : disks[idx])
        {
            FlexDirEntry dirEntry;

            if (!disk)
            {
                continue;
            }

            std::string wildcard = "TEST*.BIN";
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            EXPECT_THAT([&](){ disk->DeleteFile(wildcard); },
                    testing::Throws<FlexException>());
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            std::string filename = "TEST02.TXT";
            EXPECT_TRUE(disk->FindFile(filename, dirEntry));
            EXPECT_THAT([&](){ disk->RenameFile(filename, "TEST90.BIN"); },
                    testing::Throws<FlexException>());
            EXPECT_TRUE(disk->FindFile(filename, dirEntry));
            filename = "TEST03.TXT";
            auto attr = FLX_READONLY;
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            EXPECT_EQ(dirEntry.GetAttributes(), 0);
            EXPECT_THAT([&](){ disk->SetAttributes(filename, attr, 0); },
                    testing::Throws<FlexException>());
            EXPECT_TRUE(disk->FindFile(wildcard, dirEntry));
            EXPECT_EQ(dirEntry.GetAttributes(), 0);
        }
    }
}
