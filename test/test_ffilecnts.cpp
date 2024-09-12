// Test free functions defined in filecnts.cpp
//
#include "gtest/gtest.h"
#include "filecnts.h"


TEST(test_filecnts, fct_isValidSectorMap)
{
    SectorMap_t sectorMap{};

    EXPECT_TRUE(isValidSectorMap(sectorMap, 2 * DBPS));
    // 0 sectors, 1 expected based on fileSize
    EXPECT_FALSE(isValidSectorMap(sectorMap, 2 * DBPS + 1));
    EXPECT_FALSE(isValidSectorMap(sectorMap, 3 * DBPS - 1));
    EXPECT_FALSE(isValidSectorMap(sectorMap, 3 * DBPS));
    sectorMap[0] = 1U;
    sectorMap[1] = 1U;
    sectorMap[2] = 2U; // 2 sectors, 1 expected based on fileSize
    EXPECT_FALSE(isValidSectorMap(sectorMap, 3 * DBPS));
    sectorMap[2] = 1U;
    EXPECT_TRUE(isValidSectorMap(sectorMap, 3 * DBPS));
    sectorMap[3] = 1U; // values != 0 after all sectors found
    sectorMap[4] = 0U;
    sectorMap[5] = 0U;
    EXPECT_FALSE(isValidSectorMap(sectorMap, 3 * DBPS));
    sectorMap[3] = 0U;
    sectorMap[4] = 1U; // value != 0 after all sectors found
    EXPECT_FALSE(isValidSectorMap(sectorMap, 3 * DBPS));
    sectorMap[4] = 0U;
    sectorMap[5] = 1U; // value != 0 after all sectors found
    EXPECT_FALSE(isValidSectorMap(sectorMap, 3 * DBPS));
    for (size_t i = 0; i < sectorMap.size(); i += 3)
    {
        sectorMap[i] = 1U;
        sectorMap[i + 1] = 1U;
        sectorMap[i + 2] = 1U;
    }
    EXPECT_TRUE(isValidSectorMap(sectorMap,
                DBPS * ((sectorMap.size() / 3) + 2)));
}
