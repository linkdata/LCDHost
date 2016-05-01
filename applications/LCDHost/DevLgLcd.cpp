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
#include <QDebug>
#include <QObject>
#include <QEvent>
#include <QList>
#include <QString>
#include <QApplication>

#include "DevLgLcd.h"

#ifdef Q_WS_WIN
# ifndef UNICODE
#  error ("This isn't going to work")
# endif
# include "windows.h"
# include "wow64.h"
# include "lglcd.h"
#endif

#ifdef Q_WS_MAC
# include "lgLcdError.h"
# include "lgLcd.h"
# ifndef ERROR_FILE_NOT_FOUND
#  define ERROR_FILE_NOT_FOUND 2
# endif
#endif

#ifndef LCD_ERR
# define LCD_ERR(x) DevLgLcd::err(x,__FILE__,__LINE__)
#endif

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


class LgLcdData;

class LgLcdDevice
{
public:
    LgLcdDevice()
    {
        name = QString();
        buttonState = 0;
        buttonMap.clear();
        memset( &ctx, 0, sizeof(ctx) );
        ctx.connection = LGLCD_INVALID_CONNECTION;
        ctx.device = LGLCD_INVALID_DEVICE;
    }

    ~LgLcdDevice()
    {
        Q_ASSERT( ctx.device == LGLCD_INVALID_DEVICE );
    }

    DevLgLcd *drv;
    QString name;
    QSize size;
    bool monochrome;
    unsigned long buttonState;
    QMap<int,QString> buttonMap;
    lgLcdOpenByTypeContext ctx;
};


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

class EventLgLcdNotification : public QEvent
{
public:
    int code;
    int p1;

    EventLgLcdNotification( int a_code, int a_p1 = 0 ) : QEvent( type() ), code(a_code), p1(a_p1) {}
    static QEvent::Type type() { static QEvent::Type typeVal = static_cast<QEvent::Type>(registerEventType()); return typeVal; }
};

class EventLgLcdButton : public QEvent
{
public:
    unsigned buttons;

    EventLgLcdButton( unsigned b ) : QEvent( type() ), buttons(b) {}
    static QEvent::Type type() { static QEvent::Type typeVal = static_cast<QEvent::Type>(registerEventType()); return typeVal; }
};

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

#ifdef Q_WS_WIN
static DWORD WINAPI DevLgLcdCB(
        int connection, const PVOID pContext,
        DWORD code, DWORD p1, DWORD p2, DWORD p3, DWORD p4 )
{
    DevLgLcd *drv = static_cast<DevLgLcd *>(pContext);
    if( drv ) qApp->postEvent( drv, new EventLgLcdNotification( code, p1 ) ); // as of this writing, p2-p4 aren't used
    return 1;
}

static DWORD WINAPI DevLgLcdButtonCB(int device, DWORD dwButtons, const PVOID pContext)
{
    LgLcdDevice *drv = static_cast<LgLcdDevice *>(pContext);
    if( drv ) drv->buttonState = dwButtons;
    return 1;
}

#endif

#ifdef Q_WS_MAC
static unsigned long DevLgLcdCB(
        int connection, const void* pContext,
        unsigned long code, unsigned long p1, unsigned long p2, unsigned long p3, unsigned long p4 )
{
    Q_UNUSED(connection);
    Q_UNUSED(p2);
    Q_UNUSED(p3);
    Q_UNUSED(p4);
    DevLgLcd *drv = reinterpret_cast<DevLgLcd *>( (quintptr) pContext);
    if( drv ) qApp->postEvent( drv, new EventLgLcdNotification( code, p1 ) ); // as of this writing, p2-p4 aren't used
    return 1;
}

static unsigned long DevLgLcdButtonCB(int device, unsigned long dwButtons, const void* pContext)
{
    Q_UNUSED(device);
    LgLcdDevice *drv = reinterpret_cast<LgLcdDevice *>( (quintptr) pContext);
    if( drv ) drv->buttonState = dwButtons;
    return 1;
}

#endif


class LgLcdData
{
public:
    LgLcdData( DevLgLcd *owner );

    ~LgLcdData()
    {
#ifdef Q_WS_MAC
        if( connectContextEx.appFriendlyName )
        {
            CFRelease( connectContextEx.appFriendlyName );
            connectContextEx.appFriendlyName = NULL;
        }
#endif
    }

    lgLcdConnectContextEx connectContextEx;
    LgLcdDevice g19;
    LgLcdDevice g15;
    LgLcdDevice g13;
    LgLcdDevice z10;
};

LgLcdData::LgLcdData( DevLgLcd *owner )
{
    memset( &connectContextEx, 0, sizeof(connectContextEx) );
#ifdef Q_WS_WIN
    connectContextEx.appFriendlyName = reinterpret_cast<const WCHAR*>( qApp->applicationName().utf16() );
#endif
#ifdef Q_WS_MAC
    connectContextEx.appFriendlyName =
            CFStringCreateWithCharacters(0, reinterpret_cast<const UniChar *>(qApp->applicationName().unicode()), qApp->applicationName().length() );
#endif
    connectContextEx.isPersistent = FALSE; // Not used in 3.00+
    connectContextEx.isAutostartable = FALSE;
    connectContextEx.onConfigure.configCallback = NULL;
    connectContextEx.onConfigure.configContext = NULL;
    connectContextEx.connection = LGLCD_INVALID_CONNECTION;
    connectContextEx.dwAppletCapabilitiesSupported = LGLCD_APPLET_CAP_BW | LGLCD_APPLET_CAP_QVGA;
    connectContextEx.onNotify.notificationCallback = DevLgLcdCB;
    connectContextEx.onNotify.notifyContext = (void*) owner;

    g19.drv = owner;
    g19.name = "Logitech G19";
    g19.size = QSize(320,240);
    g19.monochrome = false;
    g19.ctx.deviceType = LGLCD_DEVICE_QVGA;
    g19.ctx.onSoftbuttonsChanged.softbuttonsChangedCallback = DevLgLcdButtonCB;
    g19.ctx.onSoftbuttonsChanged.softbuttonsChangedContext = (void*) &g19;
    g19.buttonMap.insert( 0x0100, QObject::tr("Left") );
    g19.buttonMap.insert( 0x0200, QObject::tr("Right") );
    g19.buttonMap.insert( 0x0400, QObject::tr("Ok") );
    g19.buttonMap.insert( 0x0800, QObject::tr("Cancel") );
    g19.buttonMap.insert( 0x1000, QObject::tr("Up") );
    g19.buttonMap.insert( 0x2000, QObject::tr("Down") );
    g19.buttonMap.insert( 0x4000, QObject::tr("Menu") );

    g15.drv = owner;
    g15.name = "Logitech G15";
    g15.size = QSize(160,43);
    g15.monochrome = true;
    g15.ctx.deviceType = LGLCD_DEVICE_BW;
    g15.ctx.onSoftbuttonsChanged.softbuttonsChangedCallback = DevLgLcdButtonCB;
    g15.ctx.onSoftbuttonsChanged.softbuttonsChangedContext = (void*) &g15;
    g15.buttonMap.insert( 0x0001, QObject::tr("Softbutton 0") );
    g15.buttonMap.insert( 0x0002, QObject::tr("Softbutton 1") );
    g15.buttonMap.insert( 0x0004, QObject::tr("Softbutton 2") );
    g15.buttonMap.insert( 0x0008, QObject::tr("Softbutton 3") );

    g13.drv = owner;
    g13.name = "Logitech G13";
    g13.size = QSize(160,43);
    g13.monochrome = true;
    g13.ctx.deviceType = LGLCD_DEVICE_BW;
    g13.ctx.onSoftbuttonsChanged.softbuttonsChangedCallback = DevLgLcdButtonCB;
    g13.ctx.onSoftbuttonsChanged.softbuttonsChangedContext = (void*) &g13;
    g13.buttonMap.insert( 0x0001, QObject::tr("Softbutton 0") );
    g13.buttonMap.insert( 0x0002, QObject::tr("Softbutton 1") );
    g13.buttonMap.insert( 0x0004, QObject::tr("Softbutton 2") );
    g13.buttonMap.insert( 0x0008, QObject::tr("Softbutton 3") );

    z10.drv = owner;
    z10.name = "Logitech Z10";
    z10.size = QSize(160,43);
    z10.monochrome = true;
    z10.ctx.deviceType = LGLCD_DEVICE_BW;
    z10.ctx.onSoftbuttonsChanged.softbuttonsChangedCallback = DevLgLcdButtonCB;
    z10.ctx.onSoftbuttonsChanged.softbuttonsChangedContext = (void*) &z10;
    z10.buttonMap.insert( 0x0001, QObject::tr("Softbutton 0") );
    z10.buttonMap.insert( 0x0002, QObject::tr("Softbutton 1") );
    z10.buttonMap.insert( 0x0004, QObject::tr("Softbutton 2") );
    z10.buttonMap.insert( 0x0008, QObject::tr("Softbutton 3") );

    return;
}


DevLgLcd::DevLgLcd() : DevBase()
{
    d = new LgLcdData(this);
    if( d ) timerId = startTimer(2000); // interval to check for Logitech software running
}

DevLgLcd::~DevLgLcd()
{
    if( d )
    {
        Q_ASSERT( d->connectContextEx.connection == LGLCD_INVALID_CONNECTION );
        delete d;
        d = NULL;
        killTimer( timerId );
    }
}

void DevLgLcd::run()
{
    if( d )
    {
        connect();
        exec();
        close( &d->g19 );
        close( &d->g15 );
        close( &d->g13 );
        close( &d->z10 );
        disconnect();
    }
    return;
}

bool DevLgLcd::event( QEvent * e )
{
    if( e->type() == EventLgLcdNotification::type() )
    {
        EventLgLcdNotification *notice = static_cast<EventLgLcdNotification *>(e);
        switch( notice->code )
        {
        case LGLCD_NOTIFICATION_DEVICE_ARRIVAL:
            qDebug() << "LGLCD_NOTIFICATION_DEVICE_ARRIVAL" << ( notice->p1 == LGLCD_DEVICE_BW ? "LGLCD_DEVICE_BW" : "LGLCD_DEVICE_QVGA" );
            break;
        case LGLCD_NOTIFICATION_DEVICE_REMOVAL:
            qDebug() << "LGLCD_NOTIFICATION_DEVICE_REMOVAL" << ( notice->p1 == LGLCD_DEVICE_BW ? "LGLCD_DEVICE_BW" : "LGLCD_DEVICE_QVGA" );
            break;
        case LGLCD_NOTIFICATION_APPLET_ENABLED:
            qDebug() << "LGLCD_NOTIFICATION_APPLET_ENABLED";
            break;
        case LGLCD_NOTIFICATION_APPLET_DISABLED:
            qDebug() << "LGLCD_NOTIFICATION_APPLET_DISABLED";
            break;
        case LGLCD_NOTIFICATION_CLOSE_CONNECTION:
            qDebug() << "LGLCD_NOTIFICATION_CLOSE_CONNECTION";
            break;
        default:
            qWarning() << "Unknown notification from Logitech software:" << notice->code;
            break;
        }
        return true;
    }
    else if( e->type() == EventDevOpen::type() )
    {
        EventDevOpen *ev = static_cast<EventDevOpen *>(e);
        open( ev->devid );
        return true;
    }
    else if( e->type() == EventDevRender::type() )
    {
        EventDevRender *ev = static_cast<EventDevRender *>(e);
        render( ev->devid, ev->image );
        return true;
    }
    else if( e->type() == EventDevClose::type() )
    {
        EventDevClose *ev = static_cast<EventDevClose *>(e);
        close( ev->devid );
        return true;
    }

    return DevBase::event( e );
}

void DevLgLcd::timerEvent( QTimerEvent * event )
{
    Q_UNUSED(event);
    if( d && d->connectContextEx.connection == LGLCD_INVALID_CONNECTION ) connect();
    return;
}

bool DevLgLcd::err( int result, const char *filename, int line )
{
    const char *msg;

    if( result == ERROR_SUCCESS ) return true;

    switch( result )
    {
#ifdef ERROR_FAILED
    case ERROR_FAILED:                  msg="Failed."; break;
#endif

#ifdef Q_WS_WIN
    case RPC_S_SERVER_UNAVAILABLE:	msg="Logitech LCD subsystem is not available."; break;
    case ERROR_OLD_WIN_VERSION:		msg="Attempted to initialize for Windows 9x."; break;
    case ERROR_NO_SYSTEM_RESOURCES:	msg="Not enough system resources."; break;
    case ERROR_ALREADY_INITIALIZED:	msg="lgLcdInit() has been called before."; break;
    case ERROR_SERVICE_NOT_ACTIVE:	msg="lgLcdInit() has not been called yet."; break;
    case ERROR_FILE_NOT_FOUND:		msg="LCDMon is not running on the system."; break;
#endif

    case ERROR_ACCESS_DENIED:           msg="Access denied."; break;
    case ERROR_CALL_NOT_IMPLEMENTED:    msg="Call not implemented."; break;
    case ERROR_LOCK_FAILED:             msg="Lock failed."; break;
    case ERROR_DEVICE_NOT_CONNECTED:    msg="Device not connected."; break;
    case ERROR_INTERNAL_ERROR:          msg="Logitech driver internal error."; break;
    case ERROR_TIMEOUT:                 msg="Timeout."; break;
    case RPC_S_PROTOCOL_ERROR:          msg="Protocol error."; break;
    case ERROR_NO_MORE_ITEMS:		msg="There are no more devices to be enumerated."; break;
    case ERROR_ALREADY_EXISTS:		msg="Already exists: Can't connect or open twice."; break;
    case ERROR_INVALID_PARAMETER:	msg="Invalid parameter."; break;
    case RPC_X_WRONG_PIPE_VERSION:	msg="LCDMon does not understand the protocol."; break;
    default:                            msg=NULL; break;
    }

    if( msg )
        qWarning( "%s(%d): '%s'\n", filename, line, msg );
    else
    {
#ifdef Q_WS_WIN
        LPWSTR lpMsgBuf;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL, result, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPWSTR) &lpMsgBuf, 0, NULL);
        qWarning( "%s(%d): LCD Error '%ls' (%x)\n", filename, line, lpMsgBuf, result );
        LocalFree(lpMsgBuf);
#else
        qWarning( "%s(%d): LCD Error %x\n", filename, line, result );
#endif
    }

    return false;
}

void DevLgLcd::connect()
{
    int retv;
    if( d->connectContextEx.connection != LGLCD_INVALID_CONNECTION ) return;
    if( lgLcdInit() == ERROR_SUCCESS )
    {
        retv = lgLcdConnectEx( & d->connectContextEx );
        if( retv != ERROR_FILE_NOT_FOUND && retv != ERROR_ALREADY_EXISTS && LCD_ERR( retv ) )
        {
            // connected, log logitech version
#ifdef Q_WS_WIN
            HKEY hKey = (HKEY)0;
            const char *logitechkey = "SOFTWARE\\Logitech\\LCD Software\\LCD Manager\\CurrentVersion";

            if( isWoW64() ) RegOpenKeyExA( HKEY_LOCAL_MACHINE, logitechkey, 0, KEY_QUERY_VALUE|KEY_WOW64_64KEY, &hKey );
            if( hKey == (HKEY)0 ) RegOpenKeyExA( HKEY_LOCAL_MACHINE, logitechkey, 0, KEY_QUERY_VALUE, &hKey );
            if( hKey != (HKEY)0 )
            {
                char logitech_version[32];
                DWORD len;
                DWORD type;
                len = sizeof(logitech_version)-1;
                if( RegQueryValueExA( hKey, "Version", NULL, &type, (LPBYTE) logitech_version, &len ) == ERROR_SUCCESS )
                {
                    logitech_version[len] = '\0';
                    if( logitech_version[0] < '3' )
                        qWarning() << "Required version of Logitech software is 3.00 or above, you have" << logitech_version;
                    else
                        qDebug() << "Logitech software version" << logitech_version;
                }
                else
                    qWarning() << "DevLgLcd: can't query Logitech software version";
            }
            else
                qWarning() << "DevLgLcd: can't open the Logitech registry key";
#endif

#ifdef Q_WS_MAC
#endif
            return;
        }
    }

    return;
}

void DevLgLcd::disconnect()
{
    if( d->connectContextEx.connection == LGLCD_INVALID_CONNECTION ) return;
    lgLcdDisconnect( d->connectContextEx.connection );
    d->connectContextEx.connection = LGLCD_INVALID_CONNECTION;
    lgLcdDeInit();
}

QList<DevId> DevLgLcd::list()
{
    QList<DevId> list;
    int retv;
    lgLcdOpenByTypeContext ctx;

    if( d->connectContextEx.connection == LGLCD_INVALID_CONNECTION ) return list;

    // TODO: When Logitech fixes the notification under OS X, we can remove this
    // and use dynamic list instead.

    // Also, they don't return ERROR_ALREADY_EXISTS, they just fail with PARAM error

    ctx.connection = d->connectContextEx.connection;
    ctx.onSoftbuttonsChanged.softbuttonsChangedCallback = DevLgLcdButtonCB;
    ctx.onSoftbuttonsChanged.softbuttonsChangedContext = (void*) this;

    // Check for QVGA
    ctx.deviceType = LGLCD_DEVICE_QVGA;
    ctx.device = LGLCD_INVALID_DEVICE;
    retv = lgLcdOpenByType( &ctx );
    if( retv == ERROR_SUCCESS || retv == ERROR_ALREADY_EXISTS )
    {
        if( retv == ERROR_SUCCESS ) lgLcdClose( ctx.device );
        list.append( DevId( d->g19.name, this, & d->g19 ) );
    }

    // Check for BW
    ctx.deviceType = LGLCD_DEVICE_BW;
    ctx.device = LGLCD_INVALID_DEVICE;
    retv = lgLcdOpenByType( &ctx );
    if( retv == ERROR_SUCCESS || retv == ERROR_ALREADY_EXISTS )
    {
        if( retv == ERROR_SUCCESS ) lgLcdClose( ctx.device );
        list.append( DevId( d->g15.name, this, & d->g15 ) );
    }

    return list;
}

const QMap<int,QString>& DevLgLcd::buttons( DevId id )
{
    static QMap<int,QString> emptyButtons;
    if( id.d ) return static_cast<LgLcdDevice*>(id.d)->buttonMap;
    return emptyButtons;
}

void DevLgLcd::open( DevId id )
{
    int retv;
    LgLcdDevice *idd = static_cast<LgLcdDevice *>(id.d);

    Q_ASSERT( id.drv == this );
    Q_ASSERT( id.d != NULL );

    if( idd == NULL || d == NULL || d->connectContextEx.connection == LGLCD_INVALID_CONNECTION ) return;
    
    if( idd->ctx.device != LGLCD_INVALID_DEVICE ) return;
    idd->ctx.connection = d->connectContextEx.connection;
    retv = lgLcdOpenByType( & idd->ctx );
    if( retv == ERROR_SUCCESS )
    {
        qDebug() << "Opened " << id.name;
        // lgLcdSetAsLCDForegroundApp( d->device, LGLCD_LCD_FOREGROUND_APP_YES );
        // lgLcdSetAsLCDForegroundApp( d->device, LGLCD_LCD_FOREGROUND_APP_NO );
    }

    return;
}

void DevLgLcd::close( LgLcdDevice *idd )
{
    if( idd == NULL || d == NULL || d->connectContextEx.connection == LGLCD_INVALID_CONNECTION ) return;

    if( idd->ctx.device != LGLCD_INVALID_DEVICE )
    {
        lgLcdClose( idd->ctx.device );
        idd->ctx.device = LGLCD_INVALID_DEVICE;
        qDebug() << "Closed " << idd->name;
    }
}

void DevLgLcd::close( DevId id )
{
    close(static_cast<LgLcdDevice *>(id.d));
    return;
}

bool DevLgLcd::monochrome( DevId id )
{
    LgLcdDevice *idd = static_cast<LgLcdDevice *>(id.d);
    if( idd == NULL ) return true;
    return idd->monochrome;
}

QSize DevLgLcd::size( DevId id )
{
    LgLcdDevice *idd = static_cast<LgLcdDevice *>(id.d);
    if( idd == NULL ) return QSize();
    return idd->size;
}

void DevLgLcd::render( DevId id, QImage image )
{
    LgLcdDevice *idd = static_cast<LgLcdDevice *>(id.d);
    int retv;
    lgLcdBitmap bm;
    const uchar *bits;

    if( idd == NULL || d == NULL || d->connectContextEx.connection == LGLCD_INVALID_CONNECTION ) return;
    if( idd->ctx.device == LGLCD_INVALID_DEVICE ) return;

    if( idd->monochrome )
    {
        uchar mask;
        int count;

        if( image.size() != idd->size )
        {
            qDebug() << "DevLgLcd::setImage() image is wrong size" << image.size();
            image = image.scaled( idd->size, Qt::IgnoreAspectRatio );
        }
        if( image.format() != QImage::Format_Mono )
        {
            image = image.convertToFormat( QImage::Format_Mono );
        }

        bm.bmp_mono.hdr.Format = LGLCD_BMP_FORMAT_160x43x1;
        bits = image.bits();
        count = 0;
        mask = 0x80;
        while( count < (160*43) )
        {
#ifdef Q_WS_MAC
            bm.bmp_mono.pixels[count] = (*bits & mask) ? 0x00 : 0xFF;
#else
            bm.bmp_mono.pixels[count] = (*bits & mask) ? 0xFF : 0x00;
#endif
            count ++;
            mask >>= 1;
            if( !mask ) { bits++; mask=0x80; }
        }
        retv = lgLcdUpdateBitmap( idd->ctx.device, &bm.hdr, LGLCD_PRIORITY_NORMAL );
    }
    else
    {
        if( image.size() != idd->size )
            image = image.scaled( idd->size, Qt::IgnoreAspectRatio );
        if( image.format() != QImage::Format_ARGB32_Premultiplied )
            image = image.convertToFormat( QImage::Format_ARGB32_Premultiplied );
        bits = image.bits();
        Q_ASSERT( image.numBytes() == sizeof( bm.bmp_qvga32.pixels ) );
        bm.bmp_qvga32.hdr.Format = LGLCD_BMP_FORMAT_QVGAx32;
        memcpy( bm.bmp_qvga32.pixels, bits, sizeof( bm.bmp_qvga32.pixels ) );
        retv = lgLcdUpdateBitmap( idd->ctx.device, &bm.hdr, LGLCD_PRIORITY_NORMAL );
    }

    if( retv != ERROR_SUCCESS )
    {
        LCD_ERR(retv);
        close(id);
        open(id);
    }

    return;
}
