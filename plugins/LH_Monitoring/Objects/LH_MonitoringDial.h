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

#ifndef LH_MONITORINGDIAL_H
#define LH_MONITORINGDIAL_H

#include "LH_Dial/LH_Dial.h"
#include <QTime>

#include "../Objects/LH_MonitoringObject.h"

class LH_MonitoringDial : public LH_Dial, public LH_MonitoringObject
{
    Q_OBJECT

protected:
    LH_Qt_int *setup_max_;
    LH_Qt_int *setup_min_;
    LH_Qt_QString *setup_minmax_hr_;

public:
    LH_MonitoringDial();
    const char *userInit();

    static lh_class *classInfo();

    void refresh();
    void updateNeedlesList(QStringList *names = NULL);

public slots:
    void updateBounds();
    void updateNeedles();

    void refreshMonitoringOptions() {
        LH_MonitoringObject::refreshMonitoringOptions();
    }
    void connectChangeEvents() {
        LH_MonitoringObject::connectChangeEvents();
        updateBounds(); updateNeedlesList();
    }
    void changeAppSelection() {
        LH_MonitoringObject::changeAppSelection();
        updateBounds(); updateNeedlesList();
    }
    void changeTypeSelection() {
        LH_MonitoringObject::changeTypeSelection();
        updateBounds(); updateNeedlesList();
    }
    void changeGroupSelection() {
        LH_MonitoringObject::changeGroupSelection();
        updateBounds(); updateNeedlesList();
    }
    void changeItemSelection() {
        LH_MonitoringObject::changeItemSelection();
        updateBounds(); updateNeedlesList();
    }
    void dataValidityChanged() {
        LH_MonitoringObject::dataValidityChanged();
    }
};

#endif // LH_MONITORINGDIAL_H
