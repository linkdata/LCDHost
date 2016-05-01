/**
  \file     LH_LgLcdMan.cpp
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

#include <QDebug>
#include <QCoreApplication>
#include <QEvent>
#include <QString>
#include <QFile>

#include "LH_LgLcdMan.h"
#include "LH_LgLcdLegacyThread.h"
#include "LH_LgLcdCallbackThread.h"
#include "EventLgLcdNotification.h"
#include "EventLgLcdButton.h"
#include "LogitechDevice.h"

LH_PLUGIN(LH_LgLcdMan)

char __lcdhostplugin_xml[] =
"<?xml version=\"1.0\"?>"
"<lcdhostplugin>"
  "<id>LgLcdMan</id>"
  "<rev>" STRINGIZE(REVISION) "</rev>"
  "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
  "<ver>" "r" STRINGIZE(REVISION) "</ver>"
  "<versionurl>http://www.linkdata.se/lcdhost/version.php?arch=$ARCH</versionurl>"
  "<author>Johan \"SirReal\" Lindh</author>"
  "<homepageurl><a href=\"http://www.linkdata.se/software/lcdhost\">Link Data Stockholm</a></homepageurl>"
  "<logourl></logourl>"
  "<shortdesc>"
  "Logitech G-series keyboard driver using the Logitech software."
  "</shortdesc>"
  "<longdesc>"
  "This driver interfaces with the Logitech LCD Manager, which is part of the Logitech G-series keyboard driver software."
  "</longdesc>"
"</lcdhostplugin>";

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

#ifdef Q_OS_WIN
#ifndef KEY_WOW64_64KEY
# define KEY_WOW64_64KEY		0x0100
# define KEY_WOW64_32KEY		0x0200
#endif

typedef BOOL (WINAPI *IsWow64Process_t)( HANDLE, PBOOL );

static int isWoW64()
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

const char *LH_LgLcdMan::userInit()
{
#ifdef Q_OS_WIN
    thread_ = new LH_LgLcdCallbackThread(this);

    // connected, log logitech version
    HKEY hKey = (HKEY)0;
    const char *logitechkey;

    logitechkey = "SOFTWARE\\Logitech\\Logitech Gaming Software\\ConfigMgr";
    if( isWoW64() ) RegOpenKeyExA( HKEY_CURRENT_USER, logitechkey, 0, KEY_QUERY_VALUE|KEY_WOW64_64KEY, &hKey );
    if( hKey == (HKEY)0 ) RegOpenKeyExA( HKEY_CURRENT_USER, logitechkey, 0, KEY_QUERY_VALUE, &hKey );
    if( hKey != (HKEY)0 )
    {
        char logitech_version[32];
        DWORD len;
        DWORD type;
        len = sizeof(logitech_version)-1;
        if( RegQueryValueExA( hKey, "RepositoryVersion", NULL, &type, (LPBYTE) logitech_version, &len ) == ERROR_SUCCESS )
        {
            logitech_version[len] = '\0';
            qDebug() << "Logitech Gaming Software version" << logitech_version;
        }
    }

    logitechkey = "SOFTWARE\\Logitech\\LCD Software\\LCD Manager\\CurrentVersion";
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
    }
#endif
#ifdef Q_OS_MAC
    thread_ = new LH_LgLcdLegacyThread( this );
#endif

    if( thread_ ) thread_->start();

    return NULL;
}

LH_LgLcdMan::~LH_LgLcdMan()
{
    if( thread_ )
    {
        thread_->timeToDie();
        thread_->quit();
        for( int waited = 0; thread_ && thread_->isRunning(); ++ waited )
        {
            QCoreApplication::processEvents();
            if( ! thread_->wait(100) && waited > 50 )
            {
                waited = 40;
                qWarning( "LH_LgLcdMan: Logitech drivers not responding" );
            }
        }
        delete thread_;
        thread_ = 0;
    }
}

void LH_LgLcdMan::userTerm()
{
    if( thread_ ) thread_->timeToDie();
    return;
}

int LH_LgLcdMan::notify(int code,void *param)
{
    Q_UNUSED(param);

    if( code && !(code&LH_NOTE_SECOND) ) return LH_NOTE_SECOND;

    if( thread_ )
    {
        if( thread_->hasBW() && bw_ == NULL )
            bw_ = new LogitechDevice( this, true );

        if( !thread_->hasBW() && bw_ != NULL )
        {
            delete bw_;
            bw_ = NULL;
        }

        if( thread_->hasQVGA() && qvga_ == NULL )
            qvga_ = new LogitechDevice( this, false );

        if( !thread_->hasQVGA() && qvga_ != NULL )
        {
            delete qvga_;
            qvga_ = NULL;
        }
    }

    return LH_NOTE_SECOND;
}

const char* LH_LgLcdMan::lglcd_Err( int result, const char *filename, unsigned line )
{
    static char uk_err_buf[64];

    if( result == ERROR_SUCCESS ) return NULL;

    switch( result )
    {
#ifdef ERROR_FAILED
    case ERROR_FAILED:                  return "Failed.";
#endif

#ifdef Q_OS_WIN
    case RPC_S_SERVER_UNAVAILABLE:	return "Logitech LCD subsystem is not available.";
    case ERROR_OLD_WIN_VERSION:		return "Attempted to initialize for Windows 9x.";
    case ERROR_NO_SYSTEM_RESOURCES:	return "Not enough system resources.";
    case ERROR_ALREADY_INITIALIZED:	return "lgLcdInit() has been called before.";
    case ERROR_SERVICE_NOT_ACTIVE:	return "lgLcdInit() has not been called yet.";
    case ERROR_FILE_NOT_FOUND:		return "LCDMon is not running on the system.";
#endif

    case ERROR_ACCESS_DENIED:           return "Access denied. LCDHost is likely disabled in the Logitech LCD Manager.";
    case ERROR_CALL_NOT_IMPLEMENTED:    return "Call not implemented.";
    case ERROR_LOCK_FAILED:             return "Lock failed.";
    case ERROR_DEVICE_NOT_CONNECTED:    return "Device not connected.";
    case ERROR_INTERNAL_ERROR:          return "Logitech driver internal error.";
    case ERROR_TIMEOUT:                 return "Timeout.";
    case RPC_S_PROTOCOL_ERROR:          return "Protocol error.";
    case ERROR_NO_MORE_ITEMS:		return "There are no more devices to be enumerated.";
    case ERROR_ALREADY_EXISTS:		return "Already exists: Can't connect or open twice.";
    case ERROR_INVALID_PARAMETER:	return "Invalid parameter.";
    case RPC_X_WRONG_PIPE_VERSION:	return "LCDMon does not understand the protocol.";
    }

#ifdef Q_OS_WIN
    LPWSTR lpMsgBuf;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL, result, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPWSTR) &lpMsgBuf, 0, NULL);
    qWarning( "LH_LgLcdMan: %s(%d) Unknown LCD error '%ls' (%x)\n",
              filename, line, lpMsgBuf, result );
    LocalFree(lpMsgBuf);
#else
    qWarning( "LH_LgLcdMan: %s(%d) Unknown LCD error %x\n", filename, line, result );
#endif

    qsnprintf(uk_err_buf, sizeof(uk_err_buf), "Error %d", result);
    return uk_err_buf;
}

bool LH_LgLcdMan::event( QEvent * e )
{
    if( e->type() == EventLgLcdButton::type() )
    {
        EventLgLcdButton *be = static_cast<EventLgLcdButton*>(e);
        if( be->bw && bw_ ) bw_->setButtonState( be->buttons );
        if( !be->bw && qvga_ ) qvga_->setButtonState( be->buttons );
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

