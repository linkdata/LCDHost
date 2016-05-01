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

#ifndef APPDRIVER_H
#define APPDRIVER_H

#if 0

#include <QObject>
#include <QList>
#include <QFileInfo>

#include "AppLibrary.h"
#include "AppDriverThread.h"

class AppDriver : public AppLibrary
{
    Q_OBJECT

public:
    AppDriver(QFileInfo fi);
    ~AppDriver();

    AppLibraryThread *libThreadCreate() { return new AppDriverThread(this,fileInfo()); }

    bool event( QEvent * );

public slots:
    void stateChanged( AppId );

signals:
    void deviceArrive();
    void deviceLeave();
};

#endif

#endif // APPDRIVER_H
