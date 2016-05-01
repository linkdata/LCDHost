/*
  Copyright (c) 2009-2016 Johan Lindh <johan@linkdata.se>

  This file is part of LCDHost.

  LCDHost is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  LCDHost is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with LCDHost.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <QtGlobal>
#include <QDateTime>
#include <QDebug>
#include "CPUEngine_win.h"

#ifdef Q_OS_WIN
# include <windows.h>
# ifdef __MINGW32__
#  include <ntdef.h>
#  include <w32api.h>

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,
    SystemProcessorInformation,
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemPathInformation,
    SystemProcessInformation,
    SystemCallCountInformation,
    SystemDeviceInformation,
    SystemProcessorPerformanceInformation,
    SystemFlagsInformation,
    SystemCallTimeInformation,
    SystemModuleInformation,
    SystemLocksInformation,
    SystemStackTraceInformation,
    SystemPagedPoolInformation,
    SystemNonPagedPoolInformation,
    SystemHandleInformation,
    SystemObjectInformation,
    SystemPageFileInformation,
    SystemVdmInstemulInformation,
    SystemVdmBopInformation,
    SystemFileCacheInformation,
    SystemPoolTagInformation,
    SystemInterruptInformation,
    SystemDpcBehaviorInformation,
    SystemFullMemoryInformation,
    SystemLoadGdiDriverInformation,
    SystemUnloadGdiDriverInformation,
    SystemTimeAdjustmentInformation,
    SystemSummaryMemoryInformation,
    SystemNextEventIdInformation,
    SystemEventIdsInformation,
    SystemCrashDumpInformation,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemKernelDebuggerInformation,
    SystemContextSwitchInformation,
    SystemRegistryQuotaInformation,
    SystemExtendServiceTableInformation,
    SystemPrioritySeperation,
    SystemPlugPlayBusInformation,
    SystemDockInformation,
    SystemPowerInformation_Nope,
    SystemProcessorSpeedInformation,
    SystemCurrentTimeZoneInformation,
    SystemLookasideInformation
} SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER Reserved1[2];
    ULONG Reserved2;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;
# else
#  include <winternl.h>
# endif
# include <psapi.h>
#endif

typedef struct _LH_SPPI // Size=48
{
    LARGE_INTEGER IdleTime; // Size=8 Offset=0
    LARGE_INTEGER KernelTime; // Size=8 Offset=8
    LARGE_INTEGER UserTime; // Size=8 Offset=16
    LARGE_INTEGER DpcTime; // Size=8 Offset=24
    LARGE_INTEGER InterruptTime; // Size=8 Offset=32
    ULONG InterruptCount; // Size=4 Offset=40
} LH_SPPI;

typedef NTSTATUS (WINAPI *NtQuerySystemInformation_t)(
  SYSTEM_INFORMATION_CLASS SystemInformationClass,
  PVOID SystemInformation,
  ULONG SystemInformationLength,
  PULONG ReturnLength
);

static NtQuerySystemInformation_t pNtQuerySystemInformation = NULL;
static HANDLE hNtDll = 0;

CPUEngine_win::CPUEngine_win()
    : CPUEngine()
    , m_sppi_ptr(0)
    , m_sppi_cb(0)
{
    if( !hNtDll ) {
        hNtDll = LoadLibraryA( "NTDLL.DLL" );
        if (!hNtDll)
            qWarning("CPUEngine_win: Can't load NTDLL.DLL");
    }
    if( hNtDll ) {
        pNtQuerySystemInformation = (NtQuerySystemInformation_t) GetProcAddress( (HMODULE) hNtDll, "NtQuerySystemInformation" );
        if( !pNtQuerySystemInformation )
            qWarning("CPUEngine_win: Can't find NtQuerySystemInformation");
    }
    count_ = getSPPI();
    return;
}

CPUEngine_win::~CPUEngine_win()
{
    m_sppi_ptr = realloc(m_sppi_ptr, m_sppi_cb = 0);
    return;
}

int CPUEngine_win::getSystemTimes()
{
    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;

    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime))
        return 0;

    if (m_sppi_cb != sizeof(LH_SPPI)) {
        m_sppi_ptr = realloc(m_sppi_ptr, sizeof(LH_SPPI));
        m_sppi_cb = m_sppi_ptr ? sizeof(LH_SPPI) : 0;
    }

    if (LH_SPPI *p_sppi = (LH_SPPI*) m_sppi_ptr) {
        memset(m_sppi_ptr, 0, m_sppi_cb);
        p_sppi->IdleTime.HighPart = idleTime.dwHighDateTime;
        p_sppi->IdleTime.LowPart = idleTime.dwLowDateTime;
        p_sppi->KernelTime.HighPart = kernelTime.dwHighDateTime;
        p_sppi->KernelTime.LowPart = kernelTime.dwLowDateTime;
        p_sppi->UserTime.HighPart = userTime.dwHighDateTime;
        p_sppi->UserTime.LowPart = userTime.dwLowDateTime;
        return 1;
    }

    return 0;
}

int CPUEngine_win::getSPPI()
{
    if (!pNtQuerySystemInformation)
        return getSystemTimes();

    ULONG retlen = 0;
    NTSTATUS status = pNtQuerySystemInformation(
            (SYSTEM_INFORMATION_CLASS) 8/*SystemProcessorPerformanceInformation*/,
            m_sppi_ptr,
            (ULONG) m_sppi_cb,
            &retlen);
    if (retlen) {
        if (retlen % sizeof(LH_SPPI))
            qFatal("CPUEngine_win: retlen %lu is not a multiple of %u", retlen, sizeof(LH_SPPI));
        if (retlen != m_sppi_cb) {
            m_sppi_ptr = realloc(m_sppi_ptr, retlen);
            m_sppi_cb = m_sppi_ptr ? retlen : 0;
            return getSPPI();
        }
    }
    if (status)
        qFatal("CPUEngine_win: NTSTATUS %ld received", (long) status);
    return (int) (m_sppi_cb / sizeof(LH_SPPI));
}

void CPUEngine_win::sample( int count, lh_cpudata *data )
{
    if (!data || count < 1)
        return;

    qint64 when = QDateTime::currentDateTime().toMSecsSinceEpoch();
    count_ = getSPPI();
    if (count > count_) {
        memset(data, 0, sizeof(lh_cpudata) * count);
        count = count_;
    }
    if (LH_SPPI *p_sppi = (LH_SPPI*) m_sppi_ptr) {
        for (int n = 0; n < count_; ++n) {
            data[n].when = when;
            data[n].idle = p_sppi[n].IdleTime.QuadPart;
            data[n].work = p_sppi[n].KernelTime.QuadPart + p_sppi[n].UserTime.QuadPart - p_sppi[n].IdleTime.QuadPart;
        }
    }

    return;
}

qreal CPUEngine_win::selfusage()
{
    FILETIME create_time, exit_time, kernel_time, user_time;
    ULARGE_INTEGER ktime, utime;

    if( GetProcessTimes( GetCurrentProcess(), &create_time, &exit_time, &kernel_time, &user_time ) )
    {
            qreal div = 10000000.0 * (qreal)count_;
            ktime.u.LowPart = kernel_time.dwLowDateTime;
            ktime.u.HighPart = kernel_time.dwHighDateTime;
            utime.u.LowPart = user_time.dwLowDateTime;
            utime.u.HighPart = user_time.dwHighDateTime;
            utime.QuadPart += ktime.QuadPart;
            return (qreal)utime.QuadPart / div;
    }

    return 0.0;
}
