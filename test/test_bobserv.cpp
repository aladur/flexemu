/*
    test_bobserv.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2025  W. Schwotzer

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
#include "bobserv.h"
#include "bobservd.h"

class TestObserver : public BObserver
{
public:
    TestObserver() = default;
    ~TestObserver() override = default;

    void UpdateFrom(NotifyId id, void *param = nullptr) override
    {
        lastId = id;
        lastParam = param;
    }

    NotifyId GetLastId() const
    {
        return lastId;
    }
    void *GetLastParam() const
    {
        return lastParam;
    }

private:
    NotifyId lastId{};
    void *lastParam{nullptr};
};

class TestObserved : public BObserved
{
public:
    TestObserved() = default;
    ~TestObserved() override = default;

    void DoNotify(NotifyId id, void *param = nullptr)
    {
        Notify(id, param);
    }
};

TEST(test_bobserv, fct_notify)
{
    TestObserver observer1;
    TestObserver observer2;
    TestObserved observed;

    // Cast from integer literal is needed here.
    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast,
    // google-readability-casting)
    observed.Attach(observer1);
    observed.DoNotify(NotifyId::UpdateStatusBar, (void *)0x55AA);
    EXPECT_EQ(observer1.GetLastId(), NotifyId::UpdateStatusBar);
    EXPECT_EQ(observer1.GetLastParam(), (void *)0x55AA);
    EXPECT_NE(observer2.GetLastId(), NotifyId::UpdateStatusBar);
    EXPECT_NE(observer2.GetLastParam(), (void *)0x55AA);

    observed.Attach(observer2);
    observed.DoNotify(NotifyId::FirstKeyboardRequest, (void *)0xAA55);
    EXPECT_EQ(observer1.GetLastId(), NotifyId::FirstKeyboardRequest);
    EXPECT_EQ(observer1.GetLastParam(), (void *)0xAA55);
    EXPECT_EQ(observer2.GetLastId(), NotifyId::FirstKeyboardRequest);
    EXPECT_EQ(observer2.GetLastParam(), (void *)0xAA55);

    observed.Detach(observer1);
    observed.DoNotify(NotifyId::RequestScreenUpdate, (void *)0x2020);
    EXPECT_NE(observer1.GetLastId(), NotifyId::RequestScreenUpdate);
    EXPECT_NE(observer1.GetLastParam(), (void *)0x2020);
    EXPECT_EQ(observer2.GetLastId(), NotifyId::RequestScreenUpdate);
    EXPECT_EQ(observer2.GetLastParam(), (void *)0x2020);

    observed.Detach(observer2);
    observed.DoNotify(NotifyId::VideoRamBankChanged, (void *)0x10);
    EXPECT_NE(observer1.GetLastId(), NotifyId::VideoRamBankChanged);
    EXPECT_NE(observer1.GetLastParam(), (void *)0x10);
    EXPECT_NE(observer2.GetLastId(), NotifyId::VideoRamBankChanged);
    EXPECT_NE(observer2.GetLastParam(), (void *)0x10);
    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast,
    // google-readability-casting)
}

