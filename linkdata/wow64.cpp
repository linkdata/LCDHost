/**
  \file     wow64.cpp
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2010 Johan Lindh

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Link Data Stockholm nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
  */

#include <QtGlobal>

#ifdef Q_OS_WIN
#include "windows.h"
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

#else

int isWow64()
{
    return 0;
}

#endif
