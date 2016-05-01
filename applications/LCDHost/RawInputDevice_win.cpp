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

#include "RawInputDevice_win.h"
#include "RawInputEngine_win.h"

// #include <windows.h>
// #include <winuser.h>
// #include <api/hidsdi.h>
// #include <ddk/hidpi.h>
// #include <ddk/hidusage.h>

#if !defined(MAPVK_VK_TO_VSC)
# define MAPVK_VK_TO_VSC     (0)
# define MAPVK_VSC_TO_VK     (1)
# define MAPVK_VK_TO_CHAR    (2)
# define MAPVK_VSC_TO_VK_EX  (3)
#endif

#include <QtGlobal>
#include <QDebug>
#include <QApplication>
#include <QTime>
#include <QVector>

#include "MainWindow.h"
#include "LCDHost.h"
#include "EventRawInput.h"
#include "RawInputLogitech.h"

QVector<RawInputDevice_win*> RawInputDevice_win::vector_;

RawInputDevice_win* RawInputDevice_win::getDevice( HANDLE h )
{
    for( int i = 0; i < vector_.size(); i ++ )
        if( vector_[i]->handle() == h ) return vector_[i];
    return NULL;
}

RawInputDevice_win::RawInputDevice_win( RawInputEngine_win *e, QByteArray devicename, HANDLE h ) : RawInputDevice(e,devicename)
{
    POINT pt;
    // unsigned long ul;
    unsigned int dwSize;

    handle_ = h;
    vendor_ = 0;
    product_ = 0;
    captured_ = false;
    memset( & devinfo_, 0, sizeof(devinfo_) );
    devinfo_.cbSize = sizeof(devinfo_);
    preparsed_ = NULL;
    memset( & devcaps_, 0, sizeof(devcaps_) );
    usagemax_ = 0;
    currusagelist_ = NULL;
    prevusagelist_ = NULL;
    prevusagelen_ = 0;
    buttoncaps_ = NULL;
    valuecaps_ = NULL;
    prevvalue_ = NULL;
    prevdata_ = NULL;
    prevsize_ = 0;

    if( GetCursorPos( &pt ) )
    {
        prevmousex_ = pt.x;
        prevmousey_ = pt.y;
    }

    vector_.append( this );

    if( handle_ == 0 ) return;

    dwSize = sizeof(devinfo_);
    if( GetRawInputDeviceInfo( handle_, RIDI_DEVICEINFO, & devinfo_, & dwSize ) == (UINT) -1 )
    {
        qWarning() << "RawInputDevice_win::RawInputDevice_win(): GetRawInputDeviceInfo(RIDI_DEVICEINFO) failed: " << devicename;
        return;
    }

    vendor_ = devinfo_.dwType == RIM_TYPEHID ? devinfo_.hid.dwVendorId : 0;
    product_ = devinfo_.dwType == RIM_TYPEHID ? devinfo_.hid.dwProductId : 0;

    dwSize = 0;
    GetRawInputDeviceInfo( handle_, RIDI_PREPARSEDDATA, NULL, &dwSize );
    if( dwSize > 0 )
    {
        preparsed_ = (PHIDP_PREPARSED_DATA) (void*) new char[dwSize];
        if( GetRawInputDeviceInfo( handle_, RIDI_PREPARSEDDATA, preparsed_, &dwSize ) > 0 )
        {
            usagemax_ = HidP_MaxUsageListLength( HidP_Input, 0, preparsed_ );
            if( usagemax_ > 0xFFFF ) usagemax_ = 0;

            if( HidP_GetCaps( preparsed_, & devcaps_ ) == HIDP_STATUS_SUCCESS )
            {
                buttoncaps_ = new HIDP_BUTTON_CAPS[devcaps_.NumberInputButtonCaps];
                if( buttoncaps_ )
                {
                    HID_CAPCOUNT_T capcount = devcaps_.NumberInputButtonCaps;
                    HidP_GetButtonCaps( HidP_Input, buttoncaps_, &capcount, preparsed_ );
                    for( unsigned short i = 0; i < capcount; i ++ )
                    {
                        if( !usagepages_.contains( buttoncaps_[i].UsagePage ) ) usagepages_.append(buttoncaps_[i].UsagePage);
                        if( !buttoncaps_[i].IsRange ) buttoncount_ ++;
                        else buttoncount_ += buttoncaps_[i].Range.UsageMax - buttoncaps_[i].Range.UsageMin + 1;
                    }
                    if( usagepages_.size() )
                    {
                        currusagelist_ = new USAGE*[usagepages_.size()];
                        prevusagelist_ = new USAGE*[usagepages_.size()];
                        for( int i=0; i<usagepages_.size(); i++ )
                        {
                            if( usagemax_ )
                            {
                                currusagelist_[i] = new USAGE[usagemax_+1];
                                prevusagelist_[i] = new USAGE[usagemax_+1];
                                memset(prevusagelist_[i], 0, sizeof(USAGE)*(usagemax_+1) );
                            }
                            else
                            {
                                currusagelist_[i] = NULL;
                                prevusagelist_[i] = NULL;
                            }
                        }
                    }
                }
                valuecaps_ = new HIDP_VALUE_CAPS[devcaps_.NumberInputValueCaps];
                prevvalue_ = new ULONG*[devcaps_.NumberInputValueCaps];
                if( valuecaps_ )
                {
                    HID_CAPCOUNT_T capcount = devcaps_.NumberInputValueCaps;
                    HidP_GetValueCaps( HidP_Input, valuecaps_, &capcount, preparsed_ );
                    Q_ASSERT( devcaps_.NumberInputValueCaps == capcount );
                    for( unsigned short i = 0; i < capcount; i++ )
                    {
                        int added;
                        Q_ASSERT( valuecaps_[i].IsAbsolute );
                        if( !valuecaps_[i].IsRange ) added = 1;
                        else added = valuecaps_[i].Range.UsageMax - valuecaps_[i].Range.UsageMin + 1;
                        valuecount_ += added;
                        prevvalue_[i] = new ULONG[added];
                        memset( prevvalue_[i], 0xFE, sizeof(ULONG)*added );
                    }
                }
            }
            else
                qWarning() << "HidP_GetCaps() failed";
        }
        else
            qWarning() << "RawInput: can't get preparsed data";
    }

    if( !page() && devinfo_.dwType != RIM_TYPEHID )
    {
        devcaps_.UsagePage = 1;
        if( !usage() ) devcaps_.Usage = (devinfo_.dwType == RIM_TYPEMOUSE) ? 2 : 6;
    }

    if( devinfo_.dwType == RIM_TYPEKEYBOARD )
    {
        type_ = Keyboard;
        buttoncount_ = devinfo_.keyboard.dwNumberOfKeysTotal;
    }
    else if( devinfo_.dwType == RIM_TYPEMOUSE )
    {
        type_ = Mouse;
        valuecount_ = 2;
        buttoncount_ = devinfo_.mouse.dwNumberOfButtons;
    }
    else
    {
        if( devinfo_.hid.usUsagePage == 1 )
        {
            if( devinfo_.hid.usUsage == 4 || devinfo_.hid.usUsage == 5 ) type_ = Joystick;
            else if( devinfo_.hid.usUsage == 2 ) type_ = Mouse;
            else if( devinfo_.hid.usUsage == 6 || devinfo_.hid.usUsage == 7 ) type_ = Keyboard;
        }
        else if( devinfo_.hid.usUsagePage == 0xC )
        {
            if( devinfo_.hid.usUsage == 1 ) type_ = Keyboard; // Media keys
        }
        else
            type_ = Other;
    }

    parseName( devicename );

    devid_ = QString("%1:%2:%3:%4:%5")
          .arg( vendor(), 4, 16, QLatin1Char('0') )
          .arg( product(), 4, 16, QLatin1Char('0') )
          .arg( version(), 4, 16, QLatin1Char('0') )
          .arg( page(), 4, 16, QLatin1Char('0') )
          .arg( usage(), 4, 16, QLatin1Char('0') ).toUtf8();

#ifndef QT_NO_DEBUG
    // Dump devices
    qDebug() << QString("%1 :: %2 :: %3")
            .arg( QString::fromUtf8(devid_) )
            .arg(
                QString("%1 %2 '%3'")
                .arg( QString::fromUtf8(devicename) )
                .arg( typeName() )
                .arg( name() )
                )
            .arg(
                QString("UsageMax %1 UsagePages %2 UsageCaps %3 ValueCaps %5")
                    .arg(usagemax_)
                    .arg(usagepages_.size())
                    .arg(devcaps_.NumberInputButtonCaps)
                    .arg(devcaps_.NumberInputValueCaps)
                )
            ;
#endif

    return;
}

RawInputDevice_win::~RawInputDevice_win()
{
    int index = vector_.indexOf(this);
    if( index >= 0 ) vector_.remove( index );

    handle_ = 0;
    if( preparsed_ )
    {
        delete[] ( (char*) (void*) preparsed_ );
        preparsed_ = NULL;
    }
    if( currusagelist_ )
    {
        for( int i=0; i<usagepages_.size(); i++ )
            if( currusagelist_[i] ) delete[] currusagelist_[i];
        delete[] currusagelist_;
        currusagelist_ = NULL;
    }
    if( prevusagelist_ )
    {
        for( int i=0; i<usagepages_.size(); i++ )
            if( prevusagelist_[i] ) delete[] prevusagelist_[i];
        delete[] prevusagelist_;
        prevusagelist_ = NULL;
    }
    if( prevvalue_ )
    {
        for( unsigned i=0; i<devcaps_.NumberInputValueCaps; i++ )
        {
            if( prevvalue_[i] )
            {
                delete[] prevvalue_[i];
                prevvalue_[i] = NULL;
            }
        }
        delete[] prevvalue_;
        prevvalue_ = NULL;
    }
    if( buttoncaps_ )
    {
        delete[] buttoncaps_;
        buttoncaps_ = NULL;
    }
    if( valuecaps_ )
    {
        delete[] valuecaps_;
        valuecaps_ = NULL;
    }
    if( prevdata_ )
    {
        free( prevdata_ );
        prevdata_ = NULL;
    }
    return;
}

QString RawInputDevice_win::itemName( int item, int val, int flags )
{
    Q_UNUSED(val);

    if( page() == 0x000c && usage() == 0x0001 )
    {
        // Media keys
        switch( item & 0xFFFF )
        {
        case 0xCD: return QObject::tr("Pause/Play");
        case 0xE9: return QObject::tr("Volume Up");
        case 0xEA: return QObject::tr("Volume Down");
        case 0xB7: return QObject::tr("Stop");
        case 0xB5: return QObject::tr("Next Track");
        case 0xB6: return QObject::tr("Previous Track");
        case 0xE2: return QObject::tr("Mute");
        }
        return QString("MEDIA:%1").arg(item,4,16,QLatin1Char('0'));
    }

    if( vendor() == 0x046D )
    {
        if( flags & EventRawInput::Other && flags & EventRawInput::Button )
        {
            switch( item )
            {
            case LG_G1: return "G1";
            case LG_G2: return "G2";
            case LG_G3: return "G3";
            case LG_G4: return "G4";
            case LG_G5: return "G5";
            case LG_G6: return "G6";
            case LG_G7: return "G7";
            case LG_G8: return "G8";
            case LG_G9: return "G9";
            case LG_G10: return "G10";
            case LG_G11: return "G11";
            case LG_G12: return "G12";
            case LG_G13: return "G13";
            case LG_G14: return "G14";
            case LG_G15: return "G15";
            case LG_G16: return "G16";
            case LG_G17: return "G17";
            case LG_G18: return "G18";
            case LG_G19: return "G19";
            case LG_G20: return "G20";
            case LG_G21: return "G21";
            case LG_G22: return "G22";
            case LG_BL: return "Backlight";
            case LG_S1: return "LCD Select";
            case LG_S2: return "Softbutton 1";
            case LG_S3: return "Softbutton 2";
            case LG_S4: return "Softbutton 3";
            case LG_S5: return "Softbutton 4";
            case LG_M1: return "M1";
            case LG_M2: return "M2";
            case LG_M3: return "M3";
            case LG_MR: return "MR";
            case LG_B1: return "Stick 1";
            case LG_B2: return "Stick 2";
            case LG_B3: return "Stick 3";
            }
            return QString("LogitechButton:%1").arg(item);
        }
        if( page() == 0xff00 && usage() == 0x0001 )
        {
            if( product() == 0xC221 || product() == 0xC228 || product() == 0xC226) return "GamingMode"; // Gaming mode switch
        }
    }

    if( flags & EventRawInput::Mouse )
    {
        Q_ASSERT( isMouse() );

        if( flags & EventRawInput::Button )
            return QString::number(item);

        if( flags & EventRawInput::Neg )
        {
            switch( item )
            {
            case EventRawInput::X :     return QString("X-");
            case EventRawInput::Y :     return QString("Y-");
            case EventRawInput::Wheel : return QString("W-");
            }
        }
        else
        {
            switch( item )
            {
            case EventRawInput::X :     return QString("X+");
            case EventRawInput::Y :     return QString("Y+");
            case EventRawInput::Wheel : return QString("W+");
            }
        }
        return QString("MouseValue:%1").arg(item,4,16,QLatin1Char('0'));
    }

    if( flags & EventRawInput::Keyboard )
    {
        if( flags & EventRawInput::Button )
        {
            TCHAR namebuf[128];
            Q_ASSERT( isKeyboard() );
            if( GetKeyNameText( item, namebuf, sizeof(namebuf)/sizeof(TCHAR) ) ) return QString::fromUtf16( (ushort*) namebuf );
            return QString::number(item,16);
        }
        return QString("KeyboardValue:%1").arg(item,4,16,QLatin1Char('0'));
    }

    if( flags & EventRawInput::Joystick )
    {
        USAGE page, usage;

        page = (item >> 16) & 0xFFFF;
        usage = item & 0xFFFF;

        if( flags & EventRawInput::Button )
            return QString::number(usage);

        if( page == HID_USAGE_PAGE_GENERIC )
        {
            switch( usage )
            {
            case HID_USAGE_GENERIC_X: return QString("X");
            case HID_USAGE_GENERIC_Y: return QString("Y");
            case HID_USAGE_GENERIC_Z: return QString("Z");
            case HID_USAGE_GENERIC_RX: return QString("RX");
            case HID_USAGE_GENERIC_RY: return QString("RY");
            case HID_USAGE_GENERIC_RZ: return QString("RZ");
            case HID_USAGE_GENERIC_SLIDER: return QString("Slider");
            case HID_USAGE_GENERIC_DIAL: return QString("Dial");
            case HID_USAGE_GENERIC_WHEEL: return QString("Wheel");
            case HID_USAGE_GENERIC_HATSWITCH: return QString("Hatswitch");
            }
        }
        if( page == HID_USAGE_PAGE_SIMULATION )
        {
            switch( usage )
            {
            case HID_USAGE_SIMULATION_RUDDER: return QString("Rudder");
            case HID_USAGE_SIMULATION_THROTTLE: return QString("Throttle");
            }
        }
        if( page == HID_USAGE_PAGE_GAME )
        {
            switch( usage )
            {
            case 0x0024: return QString("Right/Left");
            case 0x0026: return QString("Up/Down");
            }
        }

        return QString("JoystickValue:%1").arg(item,8,16,QLatin1Char('0'));
    }

    return QString("%1:%2:%3").arg(page(),4,16,QLatin1Char('0')).arg(usage(),4,16,QLatin1Char('0')).arg(item,4,16,QLatin1Char('0'));
}

bool RawInputDevice_win::setCapture( bool state )
{
    captured_ = state;
    static_cast<RawInputEngine_win*>(engine_)->ensureRegistrations();
    return true;
}

void RawInputDevice_win::parseName( QString devicename )
{
    HKEY hKey;
    QString keyname;
    TCHAR name[256];
    DWORD outsize = sizeof(name);
    DWORD regtype = REG_SZ;

    if( !vendor_ ) vendor_ = devicename.mid( devicename.indexOf("#VID_")+5, 4 ).toInt(NULL,16);
    if( !product_ ) product_ = devicename.mid( devicename.indexOf("&PID_")+5, 4 ).toInt(NULL,16);

    if( !vendor_ && !product_ ) virtual_ = true;
    else virtual_ = false;

    if( vendor() == 0x046D )
    {
        switch( product() )
        {
        case 0xC228:
            if( page() == 0xff00 && usage() == 0x0001 ) { name_ = "Logitech G19 windows key switch"; valuecount_ = 1; return; }
            if( page() == 0x000c && usage() == 0x0001 ) { name_ = "Logitech G19 media keys"; return; }
            if( page() == 0x0001 && usage() == 0x0006 ) { name_ = "Logitech G19 keyboard"; return; }
            break;
        case 0xC229:
            if( page() == 0xff00 && usage() == 0x0000 )
            {
                name_ = "Logitech G19 special keys";
                buttoncount_ = 17;
                valuecount_ = 0;
                return;
            }
            break;
        case 0xC225:
            // name_ = "Logitech G11";
            break;
        case 0xC221:
            if( page() == 0xff00 && usage() == 0x0001 ) { name_ = "Logitech G15v1 windows key switch"; valuecount_ = 1; return; }
            if( page() == 0x000c && usage() == 0x0001 ) { name_ = "Logitech G15v1 media keys"; return; }
            if( page() == 0x0001 && usage() == 0x0006 ) { name_ = "Logitech G15v1 keyboard"; return; }
            break;
        case 0xC222:
            if( page() == 0xff00 && usage() == 0x0000 )
            {
                name_ = "Logitech G15v1 special keys";
                buttoncount_ = 28;
                valuecount_ = 0;
                return;
            }
            break;
        case 0xC226:
            if( page() == 0xff00 && usage() == 0x0001 ) { name_ = "Logitech G15v2 windows key switch"; valuecount_ = 1; return; }
            if( page() == 0x000c && usage() == 0x0001 ) { name_ = "Logitech G15v2 media keys"; return; }
            if( page() == 0x0001 && usage() == 0x0006 ) { name_ = "Logitech G15v2 keyboard"; return; }
            break;
        case 0xC227:
            if( page() == 0xff00 && usage() == 0x0000 )
            {
                name_ = "Logitech G15v2 special keys";
                buttoncount_ = 16;
                valuecount_ = 0;
                return;
            }
            break;
        case 0xC21C:
            name_ = "Logitech G13";
            buttoncount_ = 13 + 22;
            valuecount_ = 2;
            return;
        }
        name_ = QString("Logitech %1").arg( typeName() );
#ifndef QT_NO_DEBUG
        qDebug() << QString("Partial identification of %1 (%2/%3/%4)")
                .arg( name_ )
                .arg( product(), 4, 16, QLatin1Char('0') )
                .arg( page(), 4, 16, QLatin1Char('0') )
                .arg( usage(), 4, 16, QLatin1Char('0') );
#endif
        return;
    }

    /*
       \\?\HID#VID_046D&PID_C229&MI_01&Col02#a&203c22b&0&0001#{4d1e55b2-f16f-11cf-88cb-001111000030}
       HID\\VID_046D&PID_C229&MI_01&Col02\\a&203c22b&0&0001\\
    */

    keyname = devicename.right( devicename.size()-4 );
    for( int i = 0; i < keyname.size(); i ++ )
    {
        if( keyname[i] == '#' ) keyname[i] = '\\';
        if( keyname[i] == '{' )
        {
            keyname.truncate(i);
            break;
        }
    }
    keyname.prepend( "System\\CurrentControlSet\\Enum\\" );
    if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, (const WCHAR*) keyname.utf16(), 0, KEY_READ, &hKey) == ERROR_SUCCESS )
    {
        if( RegQueryValueEx( hKey, L"DeviceDesc", NULL, &regtype, (BYTE*) (void*) name, &outsize ) == ERROR_SUCCESS )
        {
            name[(sizeof(name)/sizeof(TCHAR))-1] = 0;
            name_ = QString::fromUtf16( (const ushort*) name );
            // Trim off Vista crap
            if( name_.at(0) == '@' )
            {
                int zap = name_.indexOf(';') + 1;
                if( zap > 0 ) name_ = name_.right( name_.size() - zap );
            }
        }
        RegCloseKey(hKey);
    }
}


void RawInputDevice_win::process( RAWINPUT *ri )
{
    int item = 0;
    LONG dx, dy;

    if( !captured_ || ri == NULL )
    {
        // Not one of the ones we want, just the same page/usage
        return;
    }

    Q_ASSERT( ri->header.hDevice == handle_ );

    // Process Logitech devices separately, since they don't respond to
    // standard HID stuff well (or at all, really)
    if( vendor() == 0x046D )
    {
        switch( product() )
        {
        case 0xC229: process_Logitech_G19( ri ); return;
        case 0xC228: break; /* standard keyboard, handle normally */
        case 0xC222: process_Logitech_G15v1( ri ); return;
        case 0xC227: process_Logitech_G15v2( ri ); return;
        case 0xC225: process_Logitech_G11( ri ); return;
        case 0xC21C: process_Logitech_G13( ri ); return;
        }
    }

    // Handle standard rawinput stuff
    switch( ri->header.dwType )
    {
    case RIM_TYPEHID:
        process_Usages(ri);
        process_Values(ri);
        break;
    case RIM_TYPEKEYBOARD:
        {
            item = ( ri->data.keyboard.VKey & 0xFFFF );
            if( item == 0x00FF )
            {
                // The 0xFF VKey is marked as 'reserved', but Vista
                // spams it around extended keys
                return;
            }
            item |= ( (ri->data.keyboard.MakeCode & 0xFF) )<<16;
            item |= (ri->data.keyboard.Flags&RI_KEY_E0) ? (1<<24) : 0;
            item |= (ri->data.keyboard.Flags&RI_KEY_E1) ? (1<<24) : 0;
            if( ri->data.keyboard.Flags & RI_KEY_BREAK )
            {
                broadcast( item, 0, EventRawInput::Button | EventRawInput::Keyboard | EventRawInput::Up );
            }
            else
            {
                broadcast( item, 0xFFFF, EventRawInput::Button | EventRawInput::Keyboard | EventRawInput::Down );
            }
        }
        return;
    case RIM_TYPEMOUSE:
        if( ri->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE )
        {
            dx = ri->data.mouse.lLastX - prevmousex_;
            dy = ri->data.mouse.lLastY - prevmousey_;
            prevmousex_ = ri->data.mouse.lLastX;
            prevmousey_ = ri->data.mouse.lLastY;
        }
        else
        {
            POINT pt;
            dx = ri->data.mouse.lLastX;
            dy = ri->data.mouse.lLastY;
            if( GetCursorPos( &pt ) )
            {
                prevmousex_ = pt.x;
                prevmousey_ = pt.y;
            }
        }
        if( dx ) broadcast( EventRawInput::X, dx, EventRawInput::Mouse | EventRawInput::Value | ((dx<0)?EventRawInput::Neg:EventRawInput::Pos) );
        if( dy ) broadcast( EventRawInput::Y, dy, EventRawInput::Mouse | EventRawInput::Value | ((dy<0)?EventRawInput::Neg:EventRawInput::Pos) );
        if( ri->data.mouse.usButtonFlags & RI_MOUSE_WHEEL )
        {
            broadcast( EventRawInput::Wheel, (short)ri->data.mouse.usButtonData, EventRawInput::Mouse | EventRawInput::Value | ((short)ri->data.mouse.usButtonData<0?EventRawInput::Neg:EventRawInput::Pos) );
        }
        if( ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN ) broadcast( 1, 0xFFFF, EventRawInput::Mouse | EventRawInput::Button | EventRawInput::Down );
        if( ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN ) broadcast( 2, 0xFFFF, EventRawInput::Mouse | EventRawInput::Button | EventRawInput::Down );
        if( ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN ) broadcast( 3, 0xFFFF, EventRawInput::Mouse | EventRawInput::Button | EventRawInput::Down );
        if( ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN ) broadcast( 4, 0xFFFF, EventRawInput::Mouse | EventRawInput::Button | EventRawInput::Down );
        if( ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN ) broadcast( 5, 0xFFFF, EventRawInput::Mouse | EventRawInput::Button | EventRawInput::Down );
        if( ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP ) broadcast( 1, 0, EventRawInput::Mouse | EventRawInput::Button | EventRawInput::Up );
        if( ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP ) broadcast( 2, 0, EventRawInput::Mouse | EventRawInput::Button | EventRawInput::Up );
        if( ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP ) broadcast( 3, 0, EventRawInput::Mouse | EventRawInput::Button | EventRawInput::Up );
        if( ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP ) broadcast( 4, 0, EventRawInput::Mouse | EventRawInput::Button | EventRawInput::Up );
        if( ri->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP ) broadcast( 5, 0, EventRawInput::Mouse | EventRawInput::Button | EventRawInput::Up );
        break;
    }

    return;
}

void RawInputDevice_win::process_Usages( RAWINPUT *ri )
{
    ULONG usagelen = usagemax_;
    int flags;

    flags = EventRawInput::Button;
    if( isJoystick() ) flags |= EventRawInput::Joystick;
    if( isMouse() ) flags |= EventRawInput::Mouse;
    if( isKeyboard() ) flags |= EventRawInput::Keyboard;

    for( int pagecount=0; pagecount<usagepages_.size(); pagecount++ )
    {
        if( usagemax_ < 1 )
        {
            if( currusagelist_[pagecount] )
            {
                delete[] currusagelist_[pagecount];
                currusagelist_[pagecount] = NULL;
            }
            usagelen = 0;
            if( HidP_GetUsages(
                    HidP_Input,
                    usagepages_[pagecount],
                    0,
                    NULL,
                    &usagelen,
                    preparsed_,
                    (CHAR*) (void*) & ri->data.hid.bRawData,
                    ri->data.hid.dwSizeHid ) != HIDP_STATUS_SUCCESS ) return;
            if( usagelen < 1 ) return;
            currusagelist_[pagecount] = new USAGE[usagelen+1];
        }

        if( HidP_GetUsages(
                HidP_Input,
                usagepages_[pagecount],
                0,
                currusagelist_[pagecount],
                &usagelen,
                preparsed_,
                (CHAR*) (void*) & ri->data.hid.bRawData,
                ri->data.hid.dwSizeHid
                ) == HIDP_STATUS_SUCCESS )
        {
            ULONG usagecount = qMax( usagelen, prevusagelen_ );
            USAGE *breaksusagelist = new USAGE[ usagecount ];
            USAGE *makesusagelist = new USAGE[ usagecount ];

            currusagelist_[pagecount][usagelen] = 0;
            if( prevusagelist_[pagecount] == NULL )
            {
                prevusagelist_[pagecount] = new USAGE[1];
                prevusagelist_[pagecount][0] = 0;
            }

            HidP_UsageListDifference(
                    prevusagelist_[pagecount],
                    currusagelist_[pagecount],
                    breaksusagelist,
                    makesusagelist,
                    usagecount
                    );

            if( usagemax_ < 1 )
            {
                if( prevusagelist_[pagecount] != NULL )
                    delete[] prevusagelist_[pagecount];
                prevusagelist_[pagecount] = currusagelist_[pagecount];
                prevusagelen_ = usagelen;
                currusagelist_[pagecount] = NULL;
            }
            else
            {
                memcpy( prevusagelist_[pagecount], currusagelist_[pagecount], usagemax_ * sizeof(USAGE) );
                prevusagelen_ = usagelen;
            }

            for( unsigned n=0; n<usagecount && breaksusagelist[n]; n++ )
                broadcast( (usagepages_[pagecount]<<16)|breaksusagelist[n], 0, flags | EventRawInput::Up );

            for( unsigned n=0; n<usagecount && makesusagelist[n]; n++ )
                broadcast( (usagepages_[pagecount]<<16)|makesusagelist[n], 0xFFFF, flags | EventRawInput::Down );
            delete[] breaksusagelist;
            delete[] makesusagelist;
        }
    }

    return;
}

void RawInputDevice_win::process_Values( RAWINPUT *ri )
{
    int flags;

    flags = EventRawInput::Value;
    if( isJoystick() ) flags |= EventRawInput::Joystick;
    if( isMouse() ) flags |= EventRawInput::Mouse;
    if( isKeyboard() ) flags |= EventRawInput::Keyboard;

    for( unsigned i = 0; i < devcaps_.NumberInputValueCaps; i ++ )
    {
        int ec;
        int j;
        USAGE usagemin, usagemax;
        ULONG usagevalue;

        if( valuecaps_[i].IsRange )
        {
            usagemin = valuecaps_[i].Range.UsageMin;
            usagemax = valuecaps_[i].Range.UsageMax;
        }
        else
        {
            usagemin = usagemax = valuecaps_[i].NotRange.Usage;
        }

        j = 0;
        for( USAGE usage = usagemin; usage<=usagemax; usage++,j++ )
        {
            usagevalue = 0;
            ec = HidP_GetUsageValue(
                    HidP_Input,
                    valuecaps_[i].UsagePage,
                    0,
                    usage,
                    &usagevalue,
                    preparsed_,
                    (CHAR*) (void*) & ri->data.hid.bRawData,
                    ri->data.hid.dwSizeHid
                    );
            if( ec == HIDP_STATUS_SUCCESS )
            {
                if( prevvalue_[i][j] != usagevalue )
                {
                    ULONG v = usagevalue;
                    if( valuecaps_[i].LogicalMax && (valuecaps_[i].LogicalMax > valuecaps_[i].LogicalMin) )
                        v = (usagevalue - valuecaps_[i].LogicalMin) * 0xFFFF / (valuecaps_[i].LogicalMax);
                    broadcast( (valuecaps_[i].UsagePage<<16)|usage, v, flags );
                    prevvalue_[i][j] = usagevalue;
                }
            }
            else
            {
                qDebug() << name() << "HidP_GetUsageValue() failed for" << usage;
            }
        }
    }

    return;
}

void RawInputDevice_win::process_Logitech_G11( RAWINPUT *ri )
{
    G15v2InputReport_Gaming *rpt, *prev;

    if( ri->data.hid.dwSizeHid != sizeof(G15v2InputReport_Gaming) )
    {
        error_ = QString("G11: Report size %1 (expected %2)").arg(ri->data.hid.dwSizeHid).arg(sizeof(G15v2InputReport_Gaming));
        return;
    }

    rpt = (G15v2InputReport_Gaming *) (void*) & ri->data.hid.bRawData;
    if( rpt->bReportId != 2 )
    {
        error_ = "G11: Report ID != 2";
        return;
    }

    if( prevdata_ == NULL )
    {
        prevdata_ = malloc( sizeof(G15v2InputReport_Gaming) );
        if( prevdata_ == NULL ) return;
        memset( prevdata_, 0, sizeof(G15v2InputReport_Gaming) );
    }

    prev = (G15v2InputReport_Gaming *) (void*) prevdata_;

#define TEST_BREAK(BN) if( prev->BN && !rpt->BN ) broadcast( LG_##BN, 0, EventRawInput::Other | EventRawInput::Button | EventRawInput::Up )
#define TEST_MAKE(BN) if( !prev->BN && rpt->BN ) broadcast( LG_##BN, 0xFFFF, EventRawInput::Other | EventRawInput::Button | EventRawInput::Down )
#define TEST_BTN(N) TEST_BREAK(N); TEST_MAKE(N);
    TEST_BTN(G1)
    TEST_BTN(G2)
    TEST_BTN(G3)
    TEST_BTN(G4)
    TEST_BTN(G5)
    TEST_BTN(G6)
    TEST_BTN(S1)
    TEST_BTN(S2)
    TEST_BTN(S3)
    TEST_BTN(S4)
    TEST_BTN(S5)
    TEST_BTN(M1)
    TEST_BTN(M2)
    TEST_BTN(M3)
    TEST_BTN(MR)
    TEST_BTN(BL)
#undef TEST_BTN
#undef TEST_MAKE
#undef TEST_BREAK

    memcpy( prevdata_, rpt, sizeof(G15v2InputReport_Gaming) );
    return;
}

void RawInputDevice_win::process_Logitech_G13( RAWINPUT *ri )
{
    int dx, dy;
    G13_InputReport_Gaming *rpt, *prev;

    if( ri->data.hid.dwSizeHid != sizeof(G13_InputReport_Gaming) )
    {
        error_ = "G13: Wrong report size";
        return;
    }

    rpt = (G13_InputReport_Gaming *) (void*) & ri->data.hid.bRawData;
    if( rpt->bReportId != 1 )
    {
        error_ = "G13: Report ID != 1";
        return;
    }

    if( prevdata_ == NULL )
    {
        prevdata_ = malloc( sizeof(G13_InputReport_Gaming) );
        if( prevdata_ == NULL ) return;
        memset( prevdata_, 0, sizeof(G13_InputReport_Gaming) );
        prev = (G13_InputReport_Gaming *) (void*) prevdata_;
        prev->BL = rpt->BL; // G13 treats BL as a toggle
    }

    prev = (G13_InputReport_Gaming *) (void*) prevdata_;

    dx = rpt->X - prev->X;
    dy = rpt->Y - prev->Y;
    if( dx ) broadcast( (HID_USAGE_PAGE_GENERIC<<16)|(HID_USAGE_GENERIC_X), (rpt->X*0xFFFF)/0xFF, EventRawInput::Value | EventRawInput::Joystick | ((dx<0)?EventRawInput::Neg:EventRawInput::Pos) );
    if( dy ) broadcast( (HID_USAGE_PAGE_GENERIC<<16)|(HID_USAGE_GENERIC_Y), (rpt->Y*0xFFFF)/0xFF, EventRawInput::Value | EventRawInput::Joystick | ((dy<0)?EventRawInput::Neg:EventRawInput::Pos) );

#define TEST_BREAK(BN) if( prev->BN && !rpt->BN ) broadcast( LG_##BN, 0, EventRawInput::Other | EventRawInput::Button | EventRawInput::Up )
#define TEST_MAKE(BN) if( !prev->BN && rpt->BN ) broadcast( LG_##BN, 0xFFFF, EventRawInput::Other | EventRawInput::Button | EventRawInput::Down )
#define TEST_BTN(N) TEST_BREAK(N); TEST_MAKE(N);
    TEST_BTN(G1)
    TEST_BTN(G2)
    TEST_BTN(G3)
    TEST_BTN(G4)
    TEST_BTN(G5)
    TEST_BTN(G6)
    TEST_BTN(G7)
    TEST_BTN(G8)
    TEST_BTN(G9)
    TEST_BTN(G10)
    TEST_BTN(G11)
    TEST_BTN(G12)
    TEST_BTN(G13)
    TEST_BTN(G14)
    TEST_BTN(G15)
    TEST_BTN(G16)
    TEST_BTN(G17)
    TEST_BTN(G18)
    TEST_BTN(G19)
    TEST_BTN(G20)
    TEST_BTN(G21)
    TEST_BTN(G22)
    TEST_BTN(BL)
    TEST_BTN(S1)
    TEST_BTN(S2)
    TEST_BTN(S3)
    TEST_BTN(S4)
    TEST_BTN(S5)
    TEST_BTN(M1)
    TEST_BTN(M2)
    TEST_BTN(M3)
    TEST_BTN(MR)
    TEST_BTN(B1)
    TEST_BTN(B2)
    TEST_BTN(B3)
#undef TEST_BTN
#undef TEST_MAKE
#undef TEST_BREAK

    memcpy( prevdata_, rpt, sizeof(G13_InputReport_Gaming) );
    return;
}

void RawInputDevice_win::process_Logitech_G15v1( RAWINPUT *ri )
{
    G15v1InputReport_Gaming *rpt, *prev;

    if( ri->data.hid.dwSizeHid != sizeof(G15v1InputReport_Gaming) )
    {
        error_ = "G15v1: Wrong report size";
        return;
    }

    rpt = (G15v1InputReport_Gaming *) (void*) & ri->data.hid.bRawData;
    if( rpt->bReportId != 2 )
    {
        error_ = "G15v1: Report ID != 2";
        return;
    }

    if( prevdata_ == NULL )
    {
        prevdata_ = malloc( sizeof(G15v1InputReport_Gaming) );
        if( prevdata_ == NULL ) return;
        memset( prevdata_, 0, sizeof(G15v1InputReport_Gaming) );
    }

    prev = (G15v1InputReport_Gaming *) (void*) prevdata_;

#define TEST_BREAK(BN) if( prev->BN && !rpt->BN ) broadcast( LG_##BN, 0, EventRawInput::Other | EventRawInput::Button | EventRawInput::Up )
#define TEST_MAKE(BN) if( !prev->BN && rpt->BN ) broadcast( LG_##BN, 0xFFFF, EventRawInput::Other | EventRawInput::Button | EventRawInput::Down )
#define TEST_BTN(N) TEST_BREAK(N); TEST_MAKE(N);
    TEST_BTN(G1)
    TEST_BTN(G2)
    TEST_BTN(G3)
    TEST_BTN(G4)
    TEST_BTN(G5)
    TEST_BTN(G6)
    TEST_BTN(G7)
    TEST_BTN(G8)
    TEST_BTN(G9)
    TEST_BTN(G10)
    TEST_BTN(G11)
    TEST_BTN(G12)
    TEST_BTN(G13)
    TEST_BTN(G14)
    TEST_BTN(G15)
    TEST_BTN(G16)
    TEST_BTN(G17)
    TEST_BTN(G18)
    TEST_BTN(S1)
    TEST_BTN(S2)
    TEST_BTN(S3)
    TEST_BTN(S4)
    TEST_BTN(S5)
    TEST_BTN(M1)
    TEST_BTN(M2)
    TEST_BTN(M3)
    TEST_BTN(MR)
    TEST_BTN(BL)
#undef TEST_BTN
#undef TEST_MAKE
#undef TEST_BREAK

    memcpy( prevdata_, rpt, sizeof(G15v1InputReport_Gaming) );
    return;
}

void RawInputDevice_win::process_Logitech_G15v2( RAWINPUT *ri )
{
    G15v2InputReport_Gaming *rpt, *prev;

    if( ri->data.hid.dwSizeHid != sizeof(G15v2InputReport_Gaming) )
    {
        error_ = QString("G15v2: Report size %1 (expected %2)").arg(ri->data.hid.dwSizeHid).arg(sizeof(G15v2InputReport_Gaming));
        return;
    }

    rpt = (G15v2InputReport_Gaming *) (void*) & ri->data.hid.bRawData;
    if( rpt->bReportId != 2 )
    {
        error_ = "G15v2: Report ID != 2";
        return;
    }

    if( prevdata_ == NULL )
    {
        prevdata_ = malloc( sizeof(G15v2InputReport_Gaming) );
        if( prevdata_ == NULL ) return;
        memset( prevdata_, 0, sizeof(G15v2InputReport_Gaming) );
    }

    prev = (G15v2InputReport_Gaming *) (void*) prevdata_;

#define TEST_BREAK(BN) if( prev->BN && !rpt->BN ) broadcast( LG_##BN, 0, EventRawInput::Other | EventRawInput::Button | EventRawInput::Up )
#define TEST_MAKE(BN) if( !prev->BN && rpt->BN ) broadcast( LG_##BN, 0xFFFF, EventRawInput::Other | EventRawInput::Button | EventRawInput::Down )
#define TEST_BTN(N) TEST_BREAK(N); TEST_MAKE(N);
    TEST_BTN(G1)
    TEST_BTN(G2)
    TEST_BTN(G3)
    TEST_BTN(G4)
    TEST_BTN(G5)
    TEST_BTN(G6)
    TEST_BTN(S1)
    TEST_BTN(S2)
    TEST_BTN(S3)
    TEST_BTN(S4)
    TEST_BTN(S5)
    TEST_BTN(M1)
    TEST_BTN(M2)
    TEST_BTN(M3)
    TEST_BTN(MR)
    TEST_BTN(BL)
#undef TEST_BTN
#undef TEST_MAKE
#undef TEST_BREAK

    memcpy( prevdata_, rpt, sizeof(G15v2InputReport_Gaming) );
    return;
}

void RawInputDevice_win::process_Logitech_G19( RAWINPUT *ri )
{
    G19InputReport_Gaming *rpt, *prev;

    if( ri->data.hid.dwSizeHid != sizeof(G19InputReport_Gaming) )
    {
        error_ = "G19: Wrong report size";
        return;
    }

    rpt = (G19InputReport_Gaming *) (void*) & ri->data.hid.bRawData;
    if( rpt->bReportId != 2 )
    {
        error_ = "G19: Report ID != 2";
        return;
    }

    if( prevdata_ == NULL )
    {
        prevdata_ = malloc( sizeof(G19InputReport_Gaming) );
        if( prevdata_ == NULL ) return;
        memset( prevdata_, 0, sizeof(G19InputReport_Gaming) );
    }

    prev = (G19InputReport_Gaming *) (void*) prevdata_;

#define TEST_BREAK(BN) if( prev->BN && !rpt->BN ) broadcast( LG_##BN, 0, EventRawInput::Other | EventRawInput::Button | EventRawInput::Up )
#define TEST_MAKE(BN) if( !prev->BN && rpt->BN ) broadcast( LG_##BN, 0xFFFF, EventRawInput::Other | EventRawInput::Button | EventRawInput::Down )
#define TEST_BTN(N) TEST_BREAK(N); TEST_MAKE(N);
    TEST_BTN(G1)
    TEST_BTN(G2)
    TEST_BTN(G3)
    TEST_BTN(G4)
    TEST_BTN(G5)
    TEST_BTN(G6)
    TEST_BTN(G7)
    TEST_BTN(G8)
    TEST_BTN(G9)
    TEST_BTN(G10)
    TEST_BTN(G11)
    TEST_BTN(G12)
    TEST_BTN(M1)
    TEST_BTN(M2)
    TEST_BTN(M3)
    TEST_BTN(MR)
    TEST_BTN(BL)
#undef TEST_BTN
#undef TEST_MAKE
#undef TEST_BREAK

    memcpy( prevdata_, rpt, sizeof(G19InputReport_Gaming) );
    return;
}

