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


#define _WIN32_WINNT 0x0501 // 0x0501 WinXP, 0x0601 WinVista
#include <windows.h>

#include "MemoryEngine_win.h"

#ifdef __MINGW32__
# include <ntdef.h>
# include <winbase.h>
/*
typedef struct _MEMORYSTATUSEX {
  DWORD     dwLength;
  DWORD     dwMemoryLoad;
  DWORDLONG ullTotalPhys;
  DWORDLONG ullAvailPhys;
  DWORDLONG ullTotalPageFile;
  DWORDLONG ullAvailPageFile;
  DWORDLONG ullTotalVirtual;
  DWORDLONG ullAvailVirtual;
  DWORDLONG ullAvailExtendedVirtual;
} MEMORYSTATUSEX, *LPMEMORYSTATUSEX;
extern "C" BOOL WINAPI GlobalMemoryStatusEx( LPMEMORYSTATUSEX lpBuffer );
*/
#endif

void MemoryEngine_win::sample( lh_memdata *p ) const
{
    MEMORYSTATUSEX state;

    if( p == NULL ) return;

    /* Get memory load */
    ZeroMemory( &state, sizeof(state) );
    state.dwLength = sizeof(state);
    if( GlobalMemoryStatusEx( &state ) )
    {
        p->tot_phys = state.ullTotalPhys;
        p->free_phys = state.ullAvailPhys;
        p->tot_virt = state.ullTotalPageFile;
        p->free_virt = state.ullAvailPageFile;
    }
    else
    {
        memset( p, 0, sizeof(lh_memdata) );
    }

    return;
}

int MemoryEngine_win::selfusage() const
{
    return 0;
}
