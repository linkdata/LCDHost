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

#ifndef LH_MONITORINGCOLORSWATCH_H
#define LH_MONITORINGCOLORSWATCH_H

#include "LH_Qt_int.h"
#include "LH_ColorSwatch/LH_ColorSwatch.h"

#include "../Objects/LH_MonitoringObject.h"

class LH_MonitoringColorSwatch : public LH_ColorSwatch, public LH_MonitoringObject
{
    Q_OBJECT

protected:
    // LH_Qt_int *setup_max_;
    // LH_Qt_int *setup_min_;

public:
    LH_MonitoringColorSwatch();
    const char *userInit();
    static lh_class *classInfo();

public slots:
    void updateValue();
    void updateBounds();

    void refreshMonitoringOptions() {
        LH_MonitoringObject::refreshMonitoringOptions();
    }
    void connectChangeEvents() {
        LH_MonitoringObject::connectChangeEvents();
        updateBounds();
    }
    void changeAppSelection() {
        LH_MonitoringObject::changeAppSelection();
        updateBounds();
    }
    void changeTypeSelection() {
        LH_MonitoringObject::changeTypeSelection();
        updateBounds();
    }
    void changeGroupSelection() {
        LH_MonitoringObject::changeGroupSelection();
        updateBounds();
    }
    void changeItemSelection() {
        LH_MonitoringObject::changeItemSelection();
        updateBounds();
    }
    void dataValidityChanged() {
        LH_MonitoringObject::dataValidityChanged();
    }
};

#endif // LH_MONITORINGCOLORSWATCH_H
