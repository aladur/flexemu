/*
    hosttime.h


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 2026  W. Schwotzer

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

#ifndef HOSTTIMER_INCLUDED
#define HOSTTIMER_INCLUDED

#include "bobserv.h"
#include "bobservd.h"
#include <cstdint>
#if defined(UNIX) && !defined(__APPLE__)
#include <ctime>
#endif
#include <functional>
#include <atomic>
#include <unordered_map>

// class HostTimer can send cyclic events with nanosecond accuracy based
// on the host clock.
class HostTimer : public BObserver, public BObserved
{
#if defined(UNIX) && !defined(__APPLE__)
    using HostTimerMap_t =
        std::unordered_map<int, std::reference_wrapper<HostTimer> >;
#endif

private:
    std::int64_t cycleTimeNs{};
    std::atomic<bool> isNotify{};
    const int uniqueTimerId{};
#if defined(UNIX) && !defined(__APPLE__)
    timer_t timerId{};
    const int sigVal{};
    int lastErrno{};
#endif

public:
    HostTimer() = delete;
    explicit HostTimer(int p_uniqueTimerId);
    ~HostTimer() override;
    HostTimer(const HostTimer &src) = delete;
    HostTimer(HostTimer &&src) = delete;
    HostTimer &operator=(const HostTimer &src) = delete;
    HostTimer &operator=(HostTimer &&src) = delete;

    void UpdateFrom(NotifyId id, void *param = nullptr) override;
    void DoNotify();
#if defined(UNIX) && !defined(__APPLE__)
    static HostTimer::HostTimerMap_t &GetHostTimerMap();
#endif

protected:
    void InitCycleTime(std::int64_t p_cycleTimeNs);
    void DisableTimer();
};

#endif

