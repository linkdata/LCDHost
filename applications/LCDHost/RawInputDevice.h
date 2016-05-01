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

#ifndef RAWINPUTDEVICE_H
#define RAWINPUTDEVICE_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <QList>

class RawInputEngine;

class RawInputDevice
{
public:
    enum Type
    {
        Keyboard,
        Mouse,
        Joystick,
        Other
    };

protected:
    RawInputEngine *engine_;
    QByteArray devid_;
    QString name_;
    QString error_;
    Type type_;
    bool captured_;
    bool virtual_;
    int buttoncount_;
    int valuecount_;
    QMutex submutex_;
    QList<QObject*> subscribers_;

public:
    RawInputDevice( RawInputEngine *e, QByteArray devid = QByteArray(), QString name = QString() ) :
            engine_(e),
            devid_(devid),
            name_(name),
            error_(QString()),
            type_(Other),
            captured_(false),
            virtual_(true),
            buttoncount_(0),
            valuecount_(0) {}
    RawInputDevice( const RawInputDevice& s ) :
            engine_(s.engine_),
            devid_(s.devid_),
            name_(s.name_),
            error_(s.error_),
            type_(s.type_),
            captured_(s.captured_),
            virtual_(s.virtual_),
            buttoncount_(s.buttoncount_),
            valuecount_(s.valuecount_) {}
    virtual ~RawInputDevice() {}

    QString name() const { return name_; }
    QByteArray devid() const { return devid_; }
    QString typeName() const;
    QString error() const { return error_; }
    Type type() const { return type_; }
    bool isValid() const { return !devid_.isNull(); }
    bool isVirtual() const { return virtual_; }
    bool isMouse() const { return type_ == Mouse; }
    bool isKeyboard() const { return type_ == Keyboard; }
    bool isJoystick() const { return type_ == Joystick; }
    bool isOther() const { return type_ == Other; }
    bool isCaptured() const { return captured_; }
    int buttonCount() const { return buttoncount_; }
    int valueCount() const { return valuecount_; }

    virtual QString itemName( int item, int val, int flags ) { Q_UNUSED(val); Q_UNUSED(flags); return QString::number(item); }
    virtual bool setCapture( bool ) { return false; }

    void broadcast( int item, int val, int flags );

    RawInputDevice& operator=(const RawInputDevice& s)
    {
        if( this != &s)
        {
            engine_ = s.engine_;
            name_ = s.name_;
            devid_ = s.devid_;
            error_ = s.error_;
            type_ = s.type_;
            captured_ = s.captured_;
            virtual_ = s.virtual_;
            buttoncount_ = s.buttoncount_;
            valuecount_ = s.valuecount_;
        }
        return *this;
    }
};

#endif // RAWINPUTDEVICE_H
