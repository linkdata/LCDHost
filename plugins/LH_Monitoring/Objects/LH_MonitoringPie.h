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

#ifndef LH_MONITORINGPIE_H
#define LH_MONITORINGPIE_H

#include "Sources/LH_MonitoringUI.h"
#include "LH_Dial/LH_Dial.h"
#include <QTime>

class LH_MonitoringPie : public LH_Dial
{
    Q_OBJECT
    QTime pollTimer_;

protected:
    LH_MonitoringUI *ui_;

public:
    LH_MonitoringPie() : LH_Dial(DIALTYPE_PIE) {ui_ = NULL;}
    const char *userInit();
    int polling();
    static lh_class *classInfo();

    void refresh();
    void updateNeedles();

public slots:
    void updateBounds();
    void configChanged();

};

#endif // LH_MONITORINGDIAL_H
