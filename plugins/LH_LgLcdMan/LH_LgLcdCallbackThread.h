/**
  \file     LH_LgLcdCallbackThread.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011, Johan Lindh

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

#ifndef LH_LGLCDCALLBACKTHREAD_H
#define LH_LGLCDCALLBACKTHREAD_H

#include "LH_LgLcdThread.h"

class LH_LgLcdCallbackThread : public LH_LgLcdThread
{
    Q_OBJECT
    int has_bw_;
    int has_qvga_;
    bool online_;

    bool render( int conn );

public:
    static lgLcdOpenByTypeContext bw_cxt;
    static lgLcdOpenByTypeContext qvga_cxt;

    explicit LH_LgLcdCallbackThread(QObject *parent = 0) : LH_LgLcdThread(parent), has_bw_(0), has_qvga_(0), online_(false) {}

    void run();

    bool hasBW() const { return has_bw_>0?true:false; }
    bool hasQVGA() const { return has_qvga_>0?true:false; }

#ifdef Q_OS_WIN
    static DWORD WINAPI LH_LogitechButtonCB(int device, DWORD dwButtons, const PVOID pContext);
#endif
#ifdef Q_OS_MAC
    static unsigned long LH_LogitechButtonCB(int device, unsigned long dwButtons, const void* pContext);
#endif

#ifdef Q_OS_WIN
    static DWORD WINAPI LH_LogitechCB( int connection, const PVOID pContext, DWORD code, DWORD p1, DWORD p2, DWORD p3, DWORD p4 );
#endif
#ifdef Q_OS_MAC
    static unsigned long LH_LogitechCB( int connection, const void* pContext, unsigned long code, unsigned long p1, unsigned long p2, unsigned long p3, unsigned long p4 );
#endif
};

#endif // LH_LGLCDCALLBACKTHREAD_H
