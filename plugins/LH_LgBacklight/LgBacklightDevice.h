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

#ifndef LGBACKLIGHTDEVICE_H
#define LGBACKLIGHTDEVICE_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QColor>

class LH_HidDevice;

typedef struct LgBacklightReport_
{
    uchar hidprefix;
    uchar bReportId;
    uchar Red;
    uchar Green;
    uchar Blue;
} LgBacklightReport;

class LgBacklightDevice : public QObject
{
    Q_OBJECT

    LH_HidDevice *hd_;
    QColor color_;
    int red_max_;
    int blue_max_;
    bool white_balance_;

public:
    LgBacklightDevice(LH_HidDevice *hd, QObject *parent);

    LH_HidDevice *hd() const { return hd_; }
    QColor color() const { return color_; }
    void setColor(QColor c);

    QColor getDeviceColor();
    bool setDeviceColor(const QColor &c);
    bool useWhiteBalance() const { return white_balance_; }
    void setUseWhiteBalance(bool b) { white_balance_ = b; }

signals:
    void colorChanged();
};

#endif // LGBACKLIGHTDEVICE_H
