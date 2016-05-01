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

#ifndef LH_MONITORINGOBJECT_H
#define LH_MONITORINGOBJECT_H

//#include <QObject>

#include "LH_QtObject.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_QString.h"
#include "LH_Qt_int.h"
#include "LH_Qt_bool.h"
#include "json.h"
#include <qmath.h>

#include "LH_MonitoringSources.h"

enum ui_mon_entry_type
{
    mon_type = 1,
    mon_group = 2,
    mon_item = 4
};

enum monitoringDataMode
{
    mdmNumbers = 1,
    mdmText = 2,
    mdmAll = mdmNumbers + mdmText,
    mdmPie = 4
};

class LH_MonitoringObject
{
    LH_QtObject* obj_;

    monitoringDataMode dataMode_;

    bool includeGroups_;
    bool isReset_;

    void updateUnitOptions();

    const char* changeAppSelectionSlot_;
    const char* changeTypeSelectionSlot_;
    const char* changeGroupSelectionSlot_;
    const char* changeItemSelectionSlot_;
    const char* refreshMonitoringOptionsSlot_;
    const char* dataValidityChangedSlot_;
    const char* renderRequiredSlot_;

    void setSelection(LH_Qt_QStringList* list_obj, LH_Qt_QString* name_obj, int et);
    void changeSelection(LH_Qt_QStringList* list_obj, LH_Qt_QString* name_obj, int et);

    bool dataValid();

    LH_Qt_QString     *setup_value_str_;
    LH_Qt_QString     *setup_value_units_;

protected:
    bool adaptiveUnitsAllowed_;

    LH_MonitoringSource* selectedMonitoringApp(bool *ok);
    SensorItem selectedSensor(bool *ok=0, int *selectedIndex=0);
    SensorGroup* selectedSensorGroup(bool *ok=0);
    QList<SensorItem> groupSensors(bool *ok=0);
    QVector<qreal> getValuesVector(bool hasDead, qreal deadVal, bool &ok, QStringList *names = NULL);

    QString value_units(bool allowAdaptation);

public:
    LH_Qt_int         *setup_value_ptr_;
    LH_Qt_bool        *setup_value_valid_;

    LH_Qt_QStringList *setup_monitoring_app_;
    LH_Qt_QString     *setup_monitoring_app_name_;

    LH_Qt_QStringList *setup_value_type_;
    LH_Qt_QString     *setup_value_type_name_;

    LH_Qt_QStringList *setup_value_group_;
    LH_Qt_QString     *setup_value_group_name_;

    LH_Qt_QStringList *setup_value_item_;
    LH_Qt_QString     *setup_value_item_name_;

    LH_Qt_QStringList *setup_unit_selection_;

    LH_Qt_bool        *setup_value_offset_;
    LH_Qt_bool        *setup_value_format_;

    LH_Qt_QString     *setup_monitoring_options_;

    QString value_str();

    double value_num(bool *ok = 0, QString *units = 0);

    QString value_units();

    QObject* value_str_obj() { return setup_value_str_;}
    QObject* value_units_obj() { return setup_value_units_;}

    LH_MonitoringObject(LH_QtObject *object, monitoringDataMode dataMode, bool includeGroups = false, bool adaptiveUnits = false);
    void monitoringInit(
            const char* refreshMonitoringOptionsSlot, const char* connectChangeEventsSlot, const char* changeAppSelectionSlot,
            const char* changeTypeSelectionSlot, const char* changeGroupSelectionSlot, const char* changeItemSelectionSlot,
            const char* dataValidityChangedSlot, const char* renderRequiredSlot);

    monitoringDataMode dataMode() { return dataMode_; }
    bool includeGroups() { return includeGroups_; }

    void setAppSelection();
    void setTypeSelection();
    void setGroupSelection();
    void setItemSelection();

    void reset();
    void clear(int);
    void refresh(int);
    void setVisible(int, bool = true);
    void setReadonly(int, bool = false);

    void showOffset(bool = true);
    void showFormat(bool = true);

    QString virtual updateLinkPaths();

    void getLinesData(DataLineCollection &dlc);

protected slots:
    void virtual connectChangeEvents();
    void virtual refreshMonitoringOptions();
    void virtual dataValidityChanged();

public slots:
    void virtual changeAppSelection();
    void virtual changeTypeSelection();
    void virtual changeGroupSelection();
    void virtual changeItemSelection();


/*

    void append(ui_mon_entry_type, QString);
    void insert(ui_mon_entry_type et, int index, QString text);
    void refresh(int);
    QString at(ui_mon_entry_type, int);
    int value(ui_mon_entry_type);
    QString valueText(ui_mon_entry_type);
    int count(ui_mon_entry_type);
    void setValue(ui_mon_entry_type, int, bool fix=false);

    bool isAverage(ui_mon_entry_type et, int sel_id_avg_) { return (value(et)==sel_id_avg_); }
    bool isGroup(ui_mon_entry_type et, int sel_id_all_) { return (value(et)==sel_id_all_); }


    bool applyOffset();
    bool applyFormat();


    void loadItemsList(int i);



    QStringList adaptiveUnitOptions()  {
        //unitOptionsType options;
        //if(getAdaptiveUnitOptions(options))
        //    return options.list();
        //else
            return QStringList();
    }

    bool setUnits(QString s) {
        return false;
        //if(!adaptiveUnitsAllowed_) return false;
        //setAdaptiveUnits(false);
        //desiredUnits_ = s;
        //return true;
    }
*/
};

#endif // LH_MONITORINGOBJECT_H
