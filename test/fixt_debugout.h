/*
    fixt_debugout.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2025  W. Schwotzer

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


#include "gtest/gtest.h"
#include <string>
#include <charconv>
#ifdef _WIN32
#include <winbase.h>
#endif


class test_DebugOutputFixture : public ::testing::Test
{
protected:
    static bool GetEnvironmentValue(const char *key, std::string &value)
    {
#ifdef _WIN32
        if (auto size = GetEnvironmentVariableA(key, nullptr, 0U); size > 0U)
        {
            value.resize(size + 1U);
            if (GetEnvironmentVariableA(key, value.data(), value.size()))
            {
                value.resize(size);
                return true;
            }
        }
#else
        if (const auto *envValue = getenv(key); envValue != nullptr)
        {
            value = envValue;
            return true;
        }
#endif
        return false;
    }

    static bool GetEnvironmentValue(const char *key, int &value)
    {
        std::string str;

        if (GetEnvironmentValue(key, str))
        {
            const auto [ptr, ec] =
                std::from_chars(str.data(), str.data() + str.size(), value);
            if (ec == std::errc() && *ptr == '\0')
            {
                return true;
            }
        }

        value = 0U;
        return false;
    }

    void SetUp() override
    {
        GetEnvironmentValue("FLEXEMU_UNITTEST_DEBUG_OUTPUT", debugOutput);
    }

    bool HasMinDebugLevel(int level) const
    {
        return debugOutput >= level;
    }

private:
    int debugOutput{0};
};
