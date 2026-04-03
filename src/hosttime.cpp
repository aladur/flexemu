/*
    hosttime.cpp


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

#include "hosttime.h"
#include "bobshelp.h"
#include <cstdint>
#include <stdexcept>
#if defined(UNIX) && !defined(__APPLE__)
#include <ctime>
#include <cerrno>
#include <csignal>
#endif
#include <functional>
#include <atomic>
#include <unordered_map>

#if defined(UNIX) && !defined(__APPLE__)
static void SignalHandler(int sig, siginfo_t *sigInfo, void *uc);
#endif

HostTimer::HostTimer(int p_uniqueTimerId)
    : uniqueTimerId(p_uniqueTimerId)
#if defined(UNIX) && !defined(__APPLE__)
    , sigVal(uniqueTimerId)
#endif
{
#if defined(UNIX) && !defined(__APPLE__)
    struct sigevent sigEvent{};
    struct sigaction sigAction{};

    sigEvent.sigev_notify = SIGEV_SIGNAL;
    sigEvent.sigev_signo = SIGRTMIN;
    sigEvent.sigev_value.sival_int = sigVal;

    if (timer_create(CLOCK_REALTIME, &sigEvent, &timerId) == -1)
    {
        lastErrno = errno;
        return;
    }

    if (GetHostTimerMap().find(sigVal) != GetHostTimerMap().cend())
    {
        throw std::invalid_argument("Timer id has to be unique");
    }

    GetHostTimerMap().insert({sigVal, std::ref(*this)});

    sigAction.sa_flags = SA_SIGINFO;
    sigAction.sa_sigaction = SignalHandler;

    sigemptyset(&sigAction.sa_mask);

    if (sigaction(SIGRTMIN, &sigAction, nullptr) == -1)
    {
        lastErrno = errno;
    }
#endif
}

HostTimer::~HostTimer()
{
    DisableTimer();
#if defined(UNIX) && !defined(__APPLE__)
    timer_delete(timerId);
    GetHostTimerMap().erase(sigVal);
#endif
}

void HostTimer::InitCycleTime(std::int64_t p_cycleTimeNs)
{
    cycleTimeNs = p_cycleTimeNs;

#if defined(UNIX) && !defined(__APPLE__)
    if (lastErrno == 0)
    {
        constexpr const std::int64_t const1SecNs = 1000000000LL;
        struct itimerspec timerSpec{};

        timerSpec.it_interval.tv_sec = p_cycleTimeNs / const1SecNs;
        timerSpec.it_interval.tv_nsec = p_cycleTimeNs % const1SecNs;
        timerSpec.it_value.tv_sec = timerSpec.it_interval.tv_sec;
        timerSpec.it_value.tv_nsec = timerSpec.it_interval.tv_nsec;

        if (timer_settime(timerId, 0, &timerSpec, nullptr) != 0)
        {
            lastErrno = errno;
        }
    }
#endif

    isNotify.store(true);
}

void HostTimer::DisableTimer()
{
    isNotify.store(false);
    cycleTimeNs = 0;

#if defined(UNIX) && !defined(__APPLE__)
    constexpr const struct itimerspec timerSpec{};

    // Disarm the timer.
    if (timer_settime(timerId, 0, &timerSpec, nullptr) == -1)
    {
        lastErrno = errno;
    }
#endif
}

void HostTimer::UpdateFrom(NotifyId id, void *param)
{
    if (id == NotifyId::SetHostTimer)
    {
        if (param == nullptr)
        {
            // A nullptr param disables all host timers.
            DisableTimer();
            return;
        }

        auto *params = reinterpret_cast<HostTimerUpdate_t *>(param);
        if (params->uniqueTimerId == uniqueTimerId)
        {
            InitCycleTime(params->cycleTimeNs);

            // Return flag if this timer is valid.
#if defined(UNIX) && !defined(__APPLE__)
            params->isValid = (lastErrno == 0);
#else
            params->isValid = false;
#endif
        }
    }
}

void HostTimer::DoNotify()
{
    if (isNotify.load())
    {
        int id{uniqueTimerId};
        Notify(NotifyId::HostTimerEvent, static_cast<void *>(&id));
    }
}

#if defined(UNIX) && !defined(__APPLE__)
HostTimer::HostTimerMap_t &HostTimer::GetHostTimerMap()
{
    static HostTimerMap_t timerForSigVal;

    return timerForSigVal;
}

static void SignalHandler(int sig, siginfo_t *sigInfo, void * /*uc*/)
{
    const int sigVal = sigInfo->si_value.sival_int;

    if (auto iter = HostTimer::GetHostTimerMap().find(sigVal);
        iter != HostTimer::GetHostTimerMap().cend() && sig == SIGRTMIN)
    {
        iter->second.get().DoNotify();
    }
}
#endif

