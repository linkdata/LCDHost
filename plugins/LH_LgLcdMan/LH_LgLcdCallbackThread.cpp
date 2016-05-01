/**
  \file     LH_LgLcdCallbackThread.cpp
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
#include <QTime>
#include <QDebug>

#include "LH_LgLcdCallbackThread.h"
#include "LH_LgLcdMan.h"
#include "EventLgLcdButton.h"
#include "EventLgLcdNotification.h"

lgLcdOpenByTypeContext LH_LgLcdCallbackThread::bw_cxt;
lgLcdOpenByTypeContext LH_LgLcdCallbackThread::qvga_cxt;

#ifdef Q_OS_WIN
DWORD WINAPI LH_LgLcdCallbackThread::LH_LogitechCB( int connection, const PVOID pContext, DWORD code, DWORD p1, DWORD p2, DWORD p3, DWORD p4 )
#endif
#ifdef Q_OS_MAC
unsigned long LH_LgLcdCallbackThread::LH_LogitechCB( int connection, const void* pContext, unsigned long code, unsigned long p1, unsigned long p2, unsigned long p3, unsigned long p4 )
#endif
{
    Q_UNUSED(connection);
    Q_UNUSED(p2);
    Q_UNUSED(p3);
    Q_UNUSED(p4);
    LH_LgLcdCallbackThread *thread = reinterpret_cast<LH_LgLcdCallbackThread*>( (quintptr) pContext);
    Q_ASSERT( thread );
    if( thread )
    {
        switch( code )
        {
        case LGLCD_NOTIFICATION_DEVICE_ARRIVAL:
            if( p1 == LGLCD_DEVICE_BW ) thread->has_bw_ ++;
            else if( p1 == LGLCD_DEVICE_QVGA ) thread->has_qvga_ ++;
            // qDebug() << "LGLCD_NOTIFICATION_DEVICE_ARRIVAL" << p1;
            break;
        case LGLCD_NOTIFICATION_DEVICE_REMOVAL:
            if( p1 == LGLCD_DEVICE_BW ) thread->has_bw_ --;
            else if( p1 == LGLCD_DEVICE_QVGA ) thread->has_qvga_ --;
            // qDebug() << "LGLCD_NOTIFICATION_DEVICE_REMOVAL" << p1;
            break;
        case LGLCD_NOTIFICATION_APPLET_ENABLED:
            // qDebug() << "LGLCD_NOTIFICATION_APPLET_ENABLED";
            break;
        case LGLCD_NOTIFICATION_APPLET_DISABLED:
            // qDebug() << "LGLCD_NOTIFICATION_APPLET_DISABLED";
            break;
        case LGLCD_NOTIFICATION_CLOSE_CONNECTION:
            // qDebug() << "LGLCD_NOTIFICATION_CLOSE_CONNECTION";
            thread->online_ = false;
            break;
        default:
            qWarning() << "Unknown notification from Logitech software:" << code;
            break;
        }
    }
    return 1;
}

#define LCD_ERR(x) ( LH_LgLcdMan::lglcd_Err((x),__FILE__,__LINE__) == NULL )

#ifdef Q_OS_WIN
DWORD WINAPI LH_LgLcdCallbackThread::LH_LogitechButtonCB(int device, DWORD dwButtons, const PVOID pContext)
#endif
#ifdef Q_OS_MAC
unsigned long LH_LgLcdCallbackThread::LH_LogitechButtonCB(int device, unsigned long dwButtons, const void* pContext)
#endif
{
    LH_LgLcdCallbackThread *thread = reinterpret_cast<LH_LgLcdCallbackThread *>( (void*) pContext );
    Q_ASSERT( device != LGLCD_INVALID_DEVICE );
    Q_ASSERT( (device == bw_cxt.device) || (device == qvga_cxt.device) );
    if( thread && thread->parent() )
        QCoreApplication::postEvent(
                thread->parent(),
                new EventLgLcdButton((bw_cxt.device == device), dwButtons),
                Qt::HighEventPriority
                );
    return 1;
}

bool LH_LgLcdCallbackThread::render( int conn )
{
    // Close open contexts no longer in use
    if( !bw_bm.hdr.Format && bw_cxt.device != LGLCD_INVALID_DEVICE )
    {
        lgLcdClose(bw_cxt.device);
        bw_cxt.device = LGLCD_INVALID_DEVICE;
    }
    if( !qvga_bm.hdr.Format && qvga_cxt.device != LGLCD_INVALID_DEVICE )
    {
        lgLcdClose(qvga_cxt.device);
        qvga_cxt.device = LGLCD_INVALID_DEVICE;
    }

    // Open context that are available and needed
    if( bw_bm.hdr.Format && bw_cxt.device == LGLCD_INVALID_DEVICE )
    {
        Q_ASSERT( bw_cxt.onSoftbuttonsChanged.softbuttonsChangedContext == this );
        bw_cxt.connection = conn;
        if( LCD_ERR( lgLcdOpenByType(&bw_cxt) ) )
            lgLcdSetAsLCDForegroundApp(bw_cxt.device,LGLCD_LCD_FOREGROUND_APP_YES);
    }
    if( qvga_bm.hdr.Format && qvga_cxt.device == LGLCD_INVALID_DEVICE )
    {
        Q_ASSERT( qvga_cxt.onSoftbuttonsChanged.softbuttonsChangedContext == this );
        qvga_cxt.connection = conn;
        if( LCD_ERR( lgLcdOpenByType(&qvga_cxt) ) )
            lgLcdSetAsLCDForegroundApp(qvga_cxt.device,LGLCD_LCD_FOREGROUND_APP_YES);
    }

    // Push bits to open devices that has images
    if( bw_cxt.device != LGLCD_INVALID_DEVICE && bw_bm.hdr.Format )
    {
        if( lgLcdUpdateBitmap( bw_cxt.device, &bw_bm.hdr, LGLCD_PRIORITY_NORMAL ) != ERROR_SUCCESS )
        {
            lgLcdClose(bw_cxt.device);
            bw_cxt.device = LGLCD_INVALID_DEVICE;
        }
    }

    if( qvga_cxt.device != LGLCD_INVALID_DEVICE && qvga_bm.hdr.Format )
    {
        if( lgLcdUpdateBitmap( qvga_cxt.device, &qvga_bm.hdr, LGLCD_PRIORITY_NORMAL ) != ERROR_SUCCESS )
        {
            lgLcdClose(qvga_cxt.device);
            qvga_cxt.device = LGLCD_INVALID_DEVICE;
        }
    }

    return true;
}

void LH_LgLcdCallbackThread::run()
{
    int index, retv;
    lgLcdConnectContextEx connectContextEx;

    bw_cxt.connection = LGLCD_INVALID_CONNECTION;
    bw_cxt.deviceType = LGLCD_DEVICE_BW;
    bw_cxt.onSoftbuttonsChanged.softbuttonsChangedCallback = LH_LogitechButtonCB;
    bw_cxt.onSoftbuttonsChanged.softbuttonsChangedContext = this;
    bw_cxt.device = LGLCD_INVALID_DEVICE;

    qvga_cxt.connection = LGLCD_INVALID_CONNECTION;
    qvga_cxt.deviceType = LGLCD_DEVICE_QVGA;
    qvga_cxt.onSoftbuttonsChanged.softbuttonsChangedCallback = LH_LogitechButtonCB;
    qvga_cxt.onSoftbuttonsChanged.softbuttonsChangedContext = this;
    qvga_cxt.device = LGLCD_INVALID_DEVICE;

    while( stayAlive() )
    {
        /* Ignore errors here to avoid spamming errors when G15 broken on not present */
        if( lgLcdInit() == ERROR_SUCCESS )
        {
            memset( &connectContextEx, 0, sizeof(connectContextEx) );
            connectContextEx.appFriendlyName = appname_;
            connectContextEx.isPersistent = 0;
            connectContextEx.isAutostartable = 0;
            connectContextEx.onConfigure.configCallback = NULL;
            connectContextEx.onConfigure.configContext = NULL;
            connectContextEx.connection = LGLCD_INVALID_CONNECTION;
            connectContextEx.dwAppletCapabilitiesSupported = LGLCD_APPLET_CAP_BW | LGLCD_APPLET_CAP_QVGA;
            connectContextEx.onNotify.notificationCallback = LH_LogitechCB;
            connectContextEx.onNotify.notifyContext = this;

            retv = lgLcdConnectEx( &connectContextEx );
            /* Don't report file not found errors */
            if( retv != -1 && retv != ERROR_FILE_NOT_FOUND && retv != ERROR_ALREADY_EXISTS && LCD_ERR( retv ) )
            {
                QTime last_enum;
                last_enum.start();
                online_ = true;

                while( stayAlive() && online_ )
                {
                    sem_.tryAcquire( 1, 100 );
                    if( !stayAlive() ) break;
                    if( !render( connectContextEx.connection ) ) break;
                }

                if( bw_cxt.device != LGLCD_INVALID_DEVICE )
                {
                    lgLcdClose( bw_cxt.device );
                    bw_cxt.device = LGLCD_INVALID_DEVICE;
                }

                if( qvga_cxt.device != LGLCD_INVALID_DEVICE )
                {
                    lgLcdClose( qvga_cxt.device );
                    qvga_cxt.device = LGLCD_INVALID_DEVICE;
                }

                if( online_ )
                {
                    retv = lgLcdDisconnect(connectContextEx.connection);
                    if( retv != ERROR_PIPE_NOT_CONNECTED ) LCD_ERR( retv );
                    online_ = false;
                }

                connectContextEx.connection = LGLCD_INVALID_CONNECTION;
            }

            lgLcdDeInit();
        }
        for( index=0; stayAlive() && index<20; index ++ ) msleep(100);
    }

    return;
}
