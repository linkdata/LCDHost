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

#ifndef LH_MONITORINGGRAPH_H
#define LH_MONITORINGGRAPH_H

#include "LH_Graph/LH_Graph.h"
#include "../Objects/LH_MonitoringObject.h"
#include "DataCollection.h"

class LH_MonitoringGraph : public LH_Graph, public LH_MonitoringObject
{
    Q_OBJECT
    bool was_empty_;

protected:
    LH_Qt_bool *setup_append_units_;

public:
    LH_MonitoringGraph(LH_QtObject* parent = 0);

    const char *userInit();
    int notify( int, void* );

    static lh_class *classInfo();
    QImage *render_qimage( int w, int h );

    void updateScale();
    void updateLines();

    void updateDataCache();

    qreal adaptToUnits(qreal val, QString *units = 0, int *prec = 0);

public slots:
    void doInitialize();
    void clearData();
    void updateUnits();
    void configChanged();

    void refreshMonitoringOptions() {
        LH_MonitoringObject::refreshMonitoringOptions();
        updateDataCache();
    }
    void connectChangeEvents() {
        LH_MonitoringObject::connectChangeEvents();
    }
    void changeAppSelection() {
        LH_MonitoringObject::changeAppSelection();
        updateDataCache();
    }
    void changeTypeSelection() {
        LH_MonitoringObject::changeTypeSelection();
        updateDataCache();
    }
    void changeGroupSelection() {
        LH_MonitoringObject::changeGroupSelection();
        updateDataCache();
    }
    void changeItemSelection() {
        LH_MonitoringObject::changeItemSelection();
        updateDataCache();
    }
    void dataValidityChanged() {
        LH_MonitoringObject::dataValidityChanged();
    }
    void renderRequired() {
        requestRender();
    }
};

#endif // LH_MONITORINGGRAPH_H
