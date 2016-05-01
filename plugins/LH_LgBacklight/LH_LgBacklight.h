/**
  \file     LH_Lg160x43.h
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

#ifndef LH_LGBACKLIGHT_H
#define LH_LGBACKLIGHT_H

#include "LH_QtPlugin.h"
#include "LH_Qt_QString.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_QColor.h"
#include "LH_Qt_int.h"

class LH_HidDevice;
class LgBacklightDevice;
class QTimerEvent;

class LH_LgBacklight : public LH_QtPlugin
{
    Q_OBJECT

    LH_Qt_QStringList *devselect_;
    LH_Qt_QColor *devcolor_;
    LH_Qt_QColor *allcolor_;
    LH_Qt_int *poll_interval_;

    QList<LgBacklightDevice *> dev_list_;

public:
    LH_LgBacklight() :
        LH_QtPlugin(),
        devselect_(0),
        devcolor_(0),
        allcolor_(0),
        poll_interval_(0)
    {}
    const char *userInit();
    void userTerm();

    void timerEvent(QTimerEvent*);

public slots:
    void onlineChanged(LH_HidDevice *hd, bool b);
    void colorChanged();
    void refreshList();
    void changeDev();
    void changeColor();
    void setAllColor();
    void pollIntervalChanged();
};

#endif // LH_LGBACKLIGHT_H
