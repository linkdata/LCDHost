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

#ifndef LH_QTPLUGIN_MAILCOUNT_H
#define LH_QTPLUGIN_MAILCOUNT_H

#include <QTime>
#include "LH_QtPlugin.h"
#include "LH_Qt_QString.h"
#include "LH_Qt_int.h"

class LH_QtPlugin_Mailcount : public LH_QtPlugin
{
    Q_OBJECT

    LH_Qt_int *email_count_;
    LH_Qt_QString *email_addr_;
    LH_Qt_int *email_days_;
    LH_Qt_int *check_interval_;
    LH_Qt_int *manual_adjust_;

    QTime last_check_;

public:
    LH_QtPlugin_Mailcount();
    ~LH_QtPlugin_Mailcount();

    virtual const char *userInit();
    virtual int notify( int code, void *param );
    virtual void userTerm();

public slots:
    void getUnreadMailcount();
};

#endif // LH_QTPLUGIN_MAILCOUNT_H
