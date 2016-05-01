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

#ifndef LH_MONITORINGTEXT_H
#define LH_MONITORINGTEXT_H

#include "LH_Text/LH_Text.h"
#include "LH_Qt_int.h"
#include "LH_Qt_float.h"
#include <QFontMetrics>
#include <QDebug>
#include "../Objects/LH_MonitoringObject.h"

class LH_MonitoringText : public LH_Text, public LH_MonitoringObject
{
    Q_OBJECT

protected:
    LH_Qt_bool *setup_value_round_;
    LH_Qt_bool *setup_append_units_;
    LH_Qt_QString *setup_pre_text_;
    LH_Qt_QString *setup_post_text_;

public:
    LH_MonitoringText();

    const char *userInit();

    static lh_class *classInfo();

public slots:
    void updateText();

    void refreshMonitoringOptions() {
        LH_MonitoringObject::refreshMonitoringOptions();
    }
    void connectChangeEvents() {
        LH_MonitoringObject::connectChangeEvents();
    }
    void changeAppSelection() {
        LH_MonitoringObject::changeAppSelection();
    }
    void changeTypeSelection() {
        LH_MonitoringObject::changeTypeSelection();
    }
    void changeGroupSelection() {
        LH_MonitoringObject::changeGroupSelection();
    }
    void changeItemSelection() {
        LH_MonitoringObject::changeItemSelection();
    }
    void dataValidityChanged() {
        LH_MonitoringObject::dataValidityChanged();
    }
};

#endif // LH_MONITORINGTEXT_H
