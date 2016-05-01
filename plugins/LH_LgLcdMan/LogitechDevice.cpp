/**
  \file     LogitechDevice.cpp
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011, Johan Lindh

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

#include <QDebug>
#include <QCoreApplication>
#include <QEvent>
#include <QString>
#include <QFile>

#include "EventLgLcdNotification.h"
#include "LogitechDevice.h"

LogitechDevice::LogitechDevice( LH_LgLcdMan *drv, bool bw ) : LH_QtDevice(drv)
{
    drv_ = drv;
    opened_ = false;
    bw_ = bw;
    buttonState_ = 0;

    if( bw_ )
    {
        setDevid( "BW" );
        setName( QObject::tr("Logitech B/W device") );
        setSize( 160, 43 );
        setDepth( 1 );
    }
    else
    {
        setDevid( "QVGA" );
        setName( QObject::tr("Logitech QVGA device") );
        setSize( 320, 240 );
        setDepth( 32 );
    }

    arrive();
    return;
}

LogitechDevice::~LogitechDevice()
{
    leave();
    if( opened() ) close();
}

void LogitechDevice::setButtonState( unsigned long button )
{
    if( buttonState_ != button )
    {
        for( unsigned long bit=0; bit<32; ++bit )
        {
            unsigned long mask = 1<<bit;
            if( (button&mask) != (buttonState_&mask) )
            {
                lh_device_input di;
                di.devid = lh_dev()->devid;
                switch( mask )
                {
                case 0x0001: di.control = "Softbutton 0"; break;
                case 0x0002: di.control = "Softbutton 1"; break;
                case 0x0004: di.control = "Softbutton 2"; break;
                case 0x0008: di.control = "Softbutton 3"; break;
                case 0x0100: di.control = "Left"; break;
                case 0x0200: di.control = "Right"; break;
                case 0x0400: di.control = "Ok"; break;
                case 0x0800: di.control = "Cancel"; break;
                case 0x1000: di.control = "Up"; break;
                case 0x2000: di.control = "Down"; break;
                case 0x4000: di.control = "Menu"; break;
                default: di.control = "Unknown"; break;
                }
                di.item = bit;
                di.flags = lh_df_button;
                if( button & mask )
                {
                    di.flags |= lh_df_down;
                    di.value = 0xFFFF;
                }
                else
                {
                    di.flags |= lh_df_up;
                    di.value = 0x0;
                }
                callback( lh_cb_input, (void*) &di );
            }
        }
        buttonState_ = button;
    }
    return;
}

const char* LogitechDevice::render_qimage(QImage *p_image)
{
    if( p_image == NULL ) return NULL;
    if( bw_ ) drv()->setBW( *p_image );
    else drv()->setQVGA( *p_image );
    return NULL;
}

const char *LogitechDevice::close()
{
    if( bw_ ) drv()->setBW( QImage() );
    else drv()->setQVGA( QImage() );
    opened_  = false;
    return NULL;
}

const char* LogitechDevice::get_backlight(lh_device_backlight*)
{
    return "get_backlight not implemented";
}

const char* LogitechDevice::set_backlight(lh_device_backlight*)
{
    return "set_backlight not implemented";
}


