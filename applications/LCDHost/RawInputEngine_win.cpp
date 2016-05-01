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
#include <QApplication>
#include <QTime>
#include <QVector>

#include "RawInputEngine_win.h"
#include "RawInputDevice_win.h"
#include "MainWindow.h"
#include "LCDHost.h"
#include "../lh_logger/LH_Logger.h"
#include "EventRawInput.h"
#include "EventRawInputRefresh.h"

HWND RawInputEngine_win::worker_window_ = 0;

LRESULT CALLBACK RawInputWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BYTE static_lpb[128];
    RawInputEngine_win *riew;
    RawInputDevice_win *dev;
    RAWINPUT *ri;
    UINT dwSize;

    switch(uMsg)
    {
    case WM_DESTROY:
        return 0;
    case WM_DEVICECHANGE:
        if( wParam == 0x0007 )
        {
            riew = static_cast<RawInputEngine_win*>((void*) GetWindowLongPtr( hWnd, GWLP_USERDATA ));
            if( riew && riew->appRawInput() ) qApp->postEvent( riew->appRawInput(), new EventRawInputRefresh() );
        }
        break;
    case WM_INPUT:
        dwSize = sizeof( static_lpb );
        if( GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, static_lpb, &dwSize, sizeof(RAWINPUTHEADER) ) != (UINT)-1 )
        {
            ri = (RAWINPUT*) static_lpb;
            dev = RawInputDevice_win::getDevice( ri->header.hDevice );
            if( dev ) dev->process( ri );
        }
        else
        {
            BYTE *lpb = NULL;
            dwSize = 0;
            GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER) );
            lpb = new BYTE[dwSize];
            qDebug() << "dynamic allocated lpb, size" << dwSize;
            if( lpb )
            {
                GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER) );
                ri = (RAWINPUT*) lpb;
                dev = RawInputDevice_win::getDevice( ri->header.hDevice );
                if( dev ) dev->process( ri );
                delete[] lpb;
            }
        }
        return 0;
    default:
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Raw input handling thread
DWORD RawInputEngineThread( void *param )
{
    RawInputEngine_win *riew = static_cast<RawInputEngine_win*>( param );
    HWND mainwindow = 0;
    WNDCLASSEX wcx;
    RECT windowsize;

    Q_ASSERT( qWinAppInst() );

    /* Create the mainwindow */
    /* Calculate wanted window size */
    windowsize.left = 0;
    windowsize.top = 0;
    windowsize.right = 40;
    windowsize.bottom = 20;
    AdjustWindowRect( &windowsize, WS_POPUPWINDOW|WS_CAPTION, FALSE );

    /* Set up our window */
    wcx.cbSize = sizeof(wcx);
    wcx.style = 0;
    wcx.lpfnWndProc = RawInputWndProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = qWinAppInst();
    wcx.hIcon = NULL;
    wcx.hCursor = (HCURSOR) LoadImage( NULL, MAKEINTRESOURCE(32512), IMAGE_CURSOR, 0,0, LR_SHARED|LR_DEFAULTSIZE );
    wcx.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wcx.lpszMenuName =  NULL;
    wcx.lpszClassName = L"LCDHostRawInputWnd";
    wcx.hIconSm = NULL;
    RegisterClassEx(&wcx);

    Q_ASSERT( wcx.hInstance );

    mainwindow = CreateWindow(
            L"LCDHostRawInputWnd", L"LCDHostRawInput",
            WS_POPUPWINDOW|WS_CAPTION,
            CW_USEDEFAULT, CW_USEDEFAULT,
            windowsize.right-windowsize.left, windowsize.bottom-windowsize.top,
            (HWND) NULL,(HMENU) NULL, wcx.hInstance, (LPVOID) NULL);

    if( mainwindow )
    {
        RAWINPUTDEVICE rid;
        MSG msg;

        RawInputEngine_win::setWorkerWindow( mainwindow );
        SetWindowLongPtr( mainwindow, GWLP_USERDATA, (quintptr) param );

        rid.usUsagePage = 0xff00;
        rid.usUsage = 0x0000;
        rid.dwFlags = RIDEV_PAGEONLY|RIDEV_INPUTSINK;
        rid.hwndTarget = mainwindow;
        if( !RegisterRawInputDevices( &rid, 1, sizeof(rid) ) ) qDebug() << "RawInput OEM monitor: can't install";

        while( !riew->timetodie() && GetMessage( &msg, mainwindow,  0, 0 ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }

        rid.dwFlags = RIDEV_PAGEONLY|RIDEV_REMOVE;
        rid.hwndTarget = 0;
        if( !RegisterRawInputDevices( &rid, 1, sizeof(rid) ) ) qDebug() << "RawInput OEM monitor: can't remove";

        DestroyWindow( mainwindow );
        mainwindow = (HWND)0;
        RawInputEngine_win::setWorkerWindow( mainwindow );
    }

    return 0;
}

RawInputEngine_win::RawInputEngine_win( AppRawInput *p ) : RawInputEngine( p )
{
    timetodie_ = false;
    worker_thread = INVALID_HANDLE_VALUE;
    worker_thread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)(RawInputEngineThread), (void*)this, 0, &worker_thread_id );
    return;
}

RawInputEngine_win::~RawInputEngine_win()
{
    timetodie_ = true;
    if( worker_thread != INVALID_HANDLE_VALUE )
    {
        if( WaitForSingleObject( worker_thread, 1000 ) != WAIT_OBJECT_0 )
            TerminateThread( worker_thread, 0 );
        CloseHandle( worker_thread );
        worker_thread = INVALID_HANDLE_VALUE;
        worker_thread_id = 0;
    }
    return;
}

const char* RawInputEngine_win::HidP_error( int status )
{
    switch( status )
    {
    case HIDP_STATUS_SUCCESS: return "HIDP_STATUS_SUCCESS";
    // case HIDP_INVALID_REPORT_LENGTH: return "HIDP_INVALID_REPORT_LENGTH";
    // case HIDP_INVALID_REPORT_TYPE: return "HIDP_INVALID_REPORT_TYPE";
    case HIDP_STATUS_BUFFER_TOO_SMALL: return "HIDP_STATUS_BUFFER_TOO_SMALL";
    case HIDP_STATUS_INCOMPATIBLE_REPORT_ID: return "HIDP_STATUS_INCOMPATIBLE_REPORT_ID";
    case HIDP_STATUS_INVALID_PREPARSED_DATA: return "HIDP_STATUS_INVALID_PREPARSED_DATA";
    case HIDP_STATUS_USAGE_NOT_FOUND: return "HIDP_STATUS_USAGE_NOT_FOUND";
    case HIDP_STATUS_BAD_LOG_PHY_VALUES: return "HIDP_STATUS_BAD_LOG_PHY_VALUES";
    case HIDP_STATUS_NULL: return "HIDP_STATUS_NULL";
    case HIDP_STATUS_VALUE_OUT_OF_RANGE: return "HIDP_STATUS_VALUE_OUT_OF_RANGE";
    }

    return "unknown code";
}

void RawInputEngine_win::clear()
{
    RawInputEngine::clear();
    ensureRegistrations();
}

void RawInputEngine_win::scan()
{
    RID_DEVICE_INFO devinfo;
    RAWINPUTDEVICELIST *devlist;
    UINT numdevices = 0;
    UINT gotten;
    unsigned int dwSize;
    char devname_buf[256];

    GetRawInputDeviceList( NULL, &numdevices, sizeof(RAWINPUTDEVICELIST) );
    if( numdevices > 0 )
    {
        QMutexLocker locker( &listmutex_ );
        devlist = new RAWINPUTDEVICELIST[numdevices];
        gotten = GetRawInputDeviceList( devlist, &numdevices, sizeof(RAWINPUTDEVICELIST) );
        for( UINT i = 0; i < gotten; i ++ )
        {
            memset( devname_buf, 0, sizeof(devname_buf) );
            dwSize = sizeof(devname_buf) / sizeof(TCHAR);
            if( GetRawInputDeviceInfoA( devlist[i].hDevice, RIDI_DEVICENAME, devname_buf, &dwSize ) > 0 )
            {
                QByteArray devname( devname_buf, dwSize );
                // Weird bug in XP
                if( devname.startsWith("\\??\\") ) devname[1] = '\\';
                // Make sure we can open the damn thing
                HANDLE hDev;
                hDev = CreateFileA(
                        devname,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0, NULL );
                if( hDev != INVALID_HANDLE_VALUE )
                {
                    CloseHandle( hDev );
                    devicelist_.append( new RawInputDevice_win( this, devname, devlist[i].hDevice ) );
                }
                else
                {
                    DWORD err = GetLastError();
                    if( err == ERROR_ACCESS_DENIED )
                    {
                        // Meh, exclusive system use thingie
                        // only allow keyboards and mice
                        devinfo.cbSize = sizeof(devinfo);
                        dwSize = sizeof(devinfo);
                        if( GetRawInputDeviceInfoA( devlist[i].hDevice, RIDI_DEVICEINFO, &devinfo, &dwSize ) > 0 )
                        {
                            if( devinfo.dwType != RIM_TYPEHID )
                            {
                                // Ugly hack to hide the bogus RawInput Logitech keyboard with 232 keys
                                // that never generates any input
                                if( devinfo.dwType == RIM_TYPEKEYBOARD && devinfo.keyboard.dwNumberOfKeysTotal < 200 )
                                    devicelist_.append( new RawInputDevice_win( this, devname, devlist[i].hDevice ) );
                                if( devinfo.dwType == RIM_TYPEMOUSE )
                                    devicelist_.append( new RawInputDevice_win( this, devname, devlist[i].hDevice) );
                            }
                        }
                    }
                    else
                    {
                        qDebug() << "Can't open device"<<devname<<":"<<LH_Logger::Win32Message(err);
                    }
                }
            }
        }
        delete[] devlist;
    }
}

void RawInputEngine_win::ensureRegistrations()
{
  AppRawInput* ri = AppRawInput::instance();
  if (!ri)
    return;
  QMutexLocker lock( &listmutex_ );
  UINT existcount = 0;
  UINT needcount = 0;
  bool found;

    RAWINPUTDEVICE *existing = new RAWINPUTDEVICE[ ri->rowCount() ];
    RAWINPUTDEVICE *needed = new RAWINPUTDEVICE[ ri->rowCount() ];

    foreach( RawInputDevice *dev, devicelist_ )
    {
        RawInputDevice_win *wd = static_cast<RawInputDevice_win *>(dev);
        if( wd->page() == 0xFF00 || !wd->isCaptured() ) continue;
        found = false;
        for( unsigned i=0; i<needcount; i++ )
        {
            if( needed[i].usUsagePage == wd->page() && needed[i].usUsage == wd->usage() )
            {
                found = true;
                break;
            }
        }
        if( found ) continue;
        needed[needcount].usUsagePage = wd->page();
        needed[needcount].usUsage = wd->usage();
        needed[needcount].dwFlags = RIDEV_INPUTSINK;
        needed[needcount].hwndTarget = worker_window_;
        needcount++;
    }

    existcount = ri->rowCount();
    if( existcount > 0 && GetRegisteredRawInputDevices( existing, &existcount, sizeof(RAWINPUTDEVICE) ) != (UINT)-1 )
    {
        for( unsigned i=0; i<existcount; i++ )
        {
            if( existing[i].usUsagePage == 0xFF00 )
            {
                existing[i].usUsagePage = 0;
                existing[i].usUsage = 0;
                existing[i].dwFlags = 0;
                existing[i].hwndTarget = 0;
            }
        }
        for( unsigned i=0; i<needcount; i++ )
        {
            found = false;
            for( unsigned j=0; j<existcount; j++ )
            {
                if( existing[j].usUsagePage == needed[i].usUsagePage &&
                    existing[j].usUsage == needed[i].usUsage )
                {
                    existing[j].usUsagePage = 0;
                    existing[j].usUsage = 0;
                    existing[j].dwFlags = 0;
                    existing[j].hwndTarget = 0;
                    found = true;
                }
            }
            if( found )
            {
                needed[i].usUsagePage = 0;
                needed[i].usUsage = 0;
                needed[i].dwFlags = 0;
                needed[i].hwndTarget = 0;
            }
        }
        for( unsigned i=0; i<existcount; i++ )
        {
            if( existing[i].usUsagePage && existing[i].usUsage )
            {
                existing[i].dwFlags = RIDEV_REMOVE;
                existing[i].hwndTarget = 0;
                if( !RegisterRawInputDevices( &existing[i], 1, sizeof(RAWINPUTDEVICE) ) )
                    qDebug() << "RawInput: can't remove page"<<existing[i].usUsagePage<<"usage"<<existing[i].usUsage<<LH_Logger::Win32Message(GetLastError());
            }
        }
    }
    else if( existcount > 0 )
    {
        qWarning() << "GetRegisteredRawInputDevices() failed:"<<LH_Logger::Win32Message(GetLastError());
    }

    for( unsigned i=0; i<needcount; i++ )
    {
        if( needed[i].usUsagePage && needed[i].usUsage )
        {
            if( !RegisterRawInputDevices( &needed[i], 1, sizeof(RAWINPUTDEVICE) ) )
                qDebug() << "RawInput: can't add page"<<needed[i].usUsagePage<<"usage"<<needed[i].usUsage<<LH_Logger::Win32Message(GetLastError());
        }
    }
    delete[] existing;
    delete[] needed;
    return;
}
