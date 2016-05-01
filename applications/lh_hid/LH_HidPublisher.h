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

#ifndef LH_HIDPUBLISHER_H
#define LH_HIDPUBLISHER_H

#include <QObject>

class LH_HidDevice;

class LH_HidPublisher : public QObject
{
    Q_OBJECT
    int publishing_;

public:
    explicit LH_HidPublisher(QObject *parent) :
        QObject(parent),
        publishing_(0)
    {}

signals:
    void onlineChanged(LH_HidDevice *hd, bool state) const;

public slots:
    void publishStart();
    void publish(LH_HidDevice *hd);
};

#endif // LH_HIDPUBLISHER_H
