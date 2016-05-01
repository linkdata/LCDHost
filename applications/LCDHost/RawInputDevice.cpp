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


#include <QDebug>
#include <QObject>
#include <QApplication>
#include <QThread>

#include "AppState.h"
#include "AppObject.h"
#include "AppLibrary.h"
#include "AppRawInput.h"
#include "EventRawInput.h"
#include "RawInputDevice.h"

QString RawInputDevice::typeName() const
{
    QString s;

    if( virtual_ ) s.append(QObject::tr("Virtual "));
    switch( type_ )
    {
    case Mouse: s.append(QObject::tr("Mouse")); break;
    case Keyboard: s.append(QObject::tr("Keyboard")); break;
    case Joystick: s.append(QObject::tr("Joystick")); break;
    case Other: s.append(QObject::tr("Device")); break;
    }

    return s;
}

void RawInputDevice::broadcast( int item, int val, int flags )
{
  if (AppRawInput* ri = AppRawInput::instance()) {
    QString control = EventRawInput::encodeControl(devid(),item,flags);
    if( QThread::currentThread() == ri->thread() )
    {
        EventRawInput evt(AppId(),devid(),control,item,val,flags);
        ri->broadcast( &evt );
    }
    else
    {
        QCoreApplication::postEvent(
              ri,
              new EventRawInput(AppId(),devid(),control,item,val,flags),
              Qt::HighEventPriority + 1);
    }
  }
  return;
}
