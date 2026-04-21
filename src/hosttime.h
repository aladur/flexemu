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

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
#define USE_POSIX_TIMERS
#endif
#endif

#include "bobserv.h"
#include "bobservd.h"
#include "fcnffile.h"
#include <cstdint>
#ifdef USE_POSIX_TIMERS
#include <ctime>
#endif
#ifdef _WIN32
#include <thread>
#include <memory>
#endif
#include <functional>
#include <atomic>
#include <unordered_map>

// class HostTimer can send cyclic events with nanosecond accuracy based
// on the host clock.
class HostTimer : public BObserver, public BObserved
{
#ifdef USE_POSIX_TIMERS
    using HostTimerMap_t =
        std::unordered_map<int, std::reference_wrapper<HostTimer> >;
#endif

private:
    volatile std::atomic<bool> isNotify{};
    const int uniqueTimerId{};
#ifdef _WIN32
    std::atomic<std::int64_t> cycleTimeNs{};
    HANDLE hTimer{};
    HANDLE hWait{};
    HANDLE hCancel{};
    DWORD lastError{};
    LARGE_INTEGER dueTime{};
    std::atomic<std::int64_t> initTicks{};
    std::unique_ptr<std::thread> timerThread;
    volatile std::atomic<bool> isFinalize{};
    bool useSpinLock{};
#else
#ifdef USE_POSIX_TIMERS
    timer_t timerId{};
    const int sigVal{};
    int lastErrno{};
#endif
#endif

public:
    HostTimer() = delete;
    HostTimer(int p_uniqueTimerId, const FlexemuConfigFileSPtr &configFile);
    ~HostTimer() override;
    HostTimer(const HostTimer &src) = delete;
    HostTimer(HostTimer &&src) = delete;
    HostTimer &operator=(const HostTimer &src) = delete;
    HostTimer &operator=(HostTimer &&src) = delete;

    void UpdateFrom(NotifyId id, void *param = nullptr) override;
    void DoNotify();
#ifdef USE_POSIX_TIMERS
    static HostTimer::HostTimerMap_t &GetHostTimerMap();
#endif

protected:
    void InitCycleTime(std::int64_t p_cycleTimeNs);
    void DisableTimer();
#ifdef _WIN32
    void Run();
#endif
};

#endif

