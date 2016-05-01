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

#ifdef Q_OS_WIN

#ifdef _WIN64
int isWoW64()
{
    return 1;
}
#else
#include <windows.h>
#include "wow64.h"

typedef BOOL (WINAPI *IsWow64Process_t)( HANDLE, PBOOL );

int isWoW64()
{
    static int is_wow64 = -1;

    if( is_wow64 == -1 )
    {
        HINSTANCE hKernel;
        IsWow64Process_t p_IsWow64Process = (IsWow64Process_t)NULL;
        is_wow64 = 0;
        hKernel = LoadLibraryA( "KERNEL32.DLL" );
        if( hKernel )
        {
            p_IsWow64Process = (IsWow64Process_t) GetProcAddress( hKernel, "IsWow64Process" );
            if( p_IsWow64Process != NULL ) p_IsWow64Process( GetCurrentProcess(), &is_wow64 );
            FreeLibrary( hKernel );
        }
    }

    return is_wow64;
}
#endif
#else

int isWow64()
{
    return 0;
}

#endif
