/**
  \file     LH_LgLcdThread.h
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

#ifndef LH_LGLCDTHREAD_H
#define LH_LGLCDTHREAD_H

#include <QThread>
#include <QImage>
#include <QSemaphore>

#ifdef Q_OS_WIN
# ifndef UNICODE
#  error ("This isn't going to work")
# endif
# include "windows.h"
// # include "../wow64.h"
# include "win/lglcd.h"
#endif

#ifdef Q_OS_MAC
# include "mac/lgLcdError.h"
# include "mac/lgLcd.h"
# ifndef ERROR_FILE_NOT_FOUND
#  define ERROR_FILE_NOT_FOUND 2
# endif
# ifndef ERROR_PIPE_NOT_CONNECTED
#  define ERROR_PIPE_NOT_CONNECTED 2
# endif
# ifndef LGLCD_DEVICE_FAMILY_ALL
#  define LGLCD_DEVICE_FAMILY_ALL 0xFF
# endif
#endif

class LH_LgLcdThread : public QThread
{
    Q_OBJECT
    bool time_to_die_;

protected:
    QSemaphore sem_;
#ifdef Q_OS_WIN
    wchar_t *appname_;
#endif
#ifdef Q_OS_MAC
    CFStringRef appname_;
#endif

public:
    explicit LH_LgLcdThread(QObject *parent = 0);
    ~LH_LgLcdThread();

    void timeToDie() { time_to_die_ = true; }
    bool stayAlive() const { return !time_to_die_; }

    virtual bool hasBW() const = 0;
    virtual bool hasQVGA() const = 0;

    void setBW( QImage img );
    void setQVGA( QImage img );

    static lgLcdBitmap bw_bm;
    static lgLcdBitmap qvga_bm;
};

#endif // LH_LGLCDTHREAD_H
