/**
  \file     LH_LgLcdThread.cpp
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

#include <QCoreApplication>
#include "LH_LgLcdThread.h"

lgLcdBitmap LH_LgLcdThread::bw_bm;
lgLcdBitmap LH_LgLcdThread::qvga_bm;

LH_LgLcdThread::LH_LgLcdThread(QObject *parent)
     : QThread(parent), time_to_die_(false), sem_(1), appname_(NULL)
{
    int appname_len = QCoreApplication::applicationName().length();
#ifdef Q_OS_WIN
    appname_ = new wchar_t[appname_len+1];
    QCoreApplication::applicationName().toWCharArray(appname_);
    appname_[appname_len] = 0;
#endif
#ifdef Q_OS_MAC
    appname_ = CFStringCreateWithCharacters(
                0, reinterpret_cast<const UniChar *>(QCoreApplication::applicationName().unicode()),
                appname_len );
#endif
}

LH_LgLcdThread::~LH_LgLcdThread()
{
    if( appname_ )
    {
#ifdef Q_OS_WIN
        delete[] (wchar_t *)appname_;
#endif
#ifdef Q_OS_MAC
        CFRelease( (CFStringRef) appname_ );
#endif
    }
}

void LH_LgLcdThread::setBW( QImage img )
{
    if( img.isNull() || img.width() != 160 || img.height() != 43 )
    {
        bw_bm.hdr.Format = 0;
        return;
    }

    bw_bm.bmp_mono.hdr.Format = LGLCD_BMP_FORMAT_160x43x1;
    for( int y=0; y<43; ++y )
    {
        for( int x=0; x<160; ++x )
        {
            Q_ASSERT( (size_t)(y*160+x) < sizeof(bw_bm.bmp_mono.pixels) );
            bw_bm.bmp_mono.pixels[y*160 + x] = ( qGray( img.pixel(x,y) ) > 128) ? 0xFF : 0x00;
        }
    }

    sem_.release();
}

void LH_LgLcdThread::setQVGA( QImage img )
{
    if( img.isNull() )
    {
        qvga_bm.hdr.Format = 0;
        return;
    }

    // Q_ASSERT( img.byteCount() == sizeof( qvga_bm.bmp_qvga32.pixels ) );

    qvga_bm.bmp_qvga32.hdr.Format = LGLCD_BMP_FORMAT_QVGAx32;
    memcpy( qvga_bm.bmp_qvga32.pixels,
#ifdef Q_OS_MAC
            img.rgbSwapped().bits(),
#else
            img.bits(),
#endif
            sizeof( qvga_bm.bmp_qvga32.pixels ) );

    sem_.release();
}
