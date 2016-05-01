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


#include "LgBacklightDevice.h"
#include "LH_LgBacklight.h"
#include "LH_HidDevice.h"
#include <QDebug>

static const int backlightid_ = 7;

LgBacklightDevice::LgBacklightDevice(LH_HidDevice *hd, QObject *parent) :
    QObject(parent),
    hd_(hd),
    color_(Qt::transparent),
    red_max_(110), // 140
    blue_max_(90), // 150
    white_balance_(true)
{
    setObjectName(hd_->objectName());
    color_ = getDeviceColor();
}

void LgBacklightDevice::setColor(QColor c)
{
    if(color_ != c && setDeviceColor(c))
        emit colorChanged();
}

QColor LgBacklightDevice::getDeviceColor()
{
    if(hd_->online())
    {
        QByteArray d(hd_->readFeature(backlightid_));
        if(d.size() > 3)
        {
            const unsigned char *report = (const unsigned char *) d.constData();
            int r = report[1];
            int g = report[2];
            int b = report[3];
            if (white_balance_) {
                int r2 = r * 255 / red_max_;
                if( r2 > 255 ) r2 = 255;
                int b2 = b * 255 / blue_max_;
                if( b2 > 255 ) b2 = 255;
                if (abs(r2 - g) < 3 && abs(b2 - g) < 3) {
                    r = r2;
                    b = b2;
                }
            }
            QColor c(r, g, b);
            return c;
        }
    }
    return QColor(Qt::transparent);
}

// automatic intensity correction
bool LgBacklightDevice::setDeviceColor(const QColor &c)
{
    if(hd_->online())
    {
        unsigned char report[5];
        int r = c.red();
        int g = c.green();
        int b = c.blue();
        if (white_balance_) {
            if (abs(r - g) < 3 && abs(g - b) < 3) {
                r = red_max_ * r / 255;
                b = blue_max_ * b / 255;
            }
        }
        report[0] = backlightid_;
        report[1] = r;
        report[2] = g;
        report[3] = b;
        report[4] = 0;
        if(hd_->writeFeature(QByteArray((const char *)report, 5)) > 0)
            return true;
    }
    return false;
}
