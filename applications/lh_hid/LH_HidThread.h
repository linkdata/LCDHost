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

#ifndef LH_HIDTHREAD_H
#define LH_HIDTHREAD_H

#include <QThread>

#if defined(LH_HID_LIBRARY)
# define LH_HID_EXPORT Q_DECL_EXPORT
#else
# define LH_HID_EXPORT Q_DECL_IMPORT
#endif

class LH_HID_EXPORT LH_HidThread : public QThread
{
    Q_OBJECT
public:
    explicit LH_HidThread(QObject *parent = 0) : QThread(parent) {}
    void run();
};

#endif // LH_HIDTHREAD_H
