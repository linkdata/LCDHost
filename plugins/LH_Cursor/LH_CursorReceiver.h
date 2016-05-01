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

#ifndef LH_CURSORRECEIVER_H
#define LH_CURSORRECEIVER_H

#include "LH_QtInstance.h"
#include "LH_Qt_QString.h"
#include "LH_Qt_QStringList.h"

#include "LH_CursorData.h"
#include <QDebug>

class LH_CursorReceiver: public QObject
{
    Q_OBJECT

public:
    LH_Qt_QString *setup_coordinate_;
    LH_Qt_QStringList *setup_cursor_state_;
    LH_Qt_QString *setup_json_data_;
    LH_Qt_QString *setup_json_postback_;

    LH_CursorReceiver(LH_QtInstance *parent, const char *amember = NULL);

    cursorData data();

    void postback(cursorData);

public slots:
    bool updateState();

signals:
    void stateChanged(bool,bool);
};

#endif // LH_CURSORRECEIVER_H

