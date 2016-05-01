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

#include "LH_MonitoringSource.h"
#include "../LH_QtMonitoringPlugin.h"

#include "json.h"
#include <QDebug>
#include <algorithm>
#include <limits>
#include <typeinfo>
#include <QVariant>

//#define LH_FLAG_HIDDEN_UNLESS_DEBUGGING LH_FLAG_LAST
#define LH_FLAG_HIDDEN_UNLESS_DEBUGGING LH_FLAG_HIDDEN

LH_MonitoringSource::LH_MonitoringSource(LH_QtObject *parent, QString appName, bool alwaysShowAllSelectors) : LH_QtObject(parent), setup_enabled_(0), sensors_()
{
    appName_ = appName;
    dataAvailable_ = false;
    alwaysShowAllSelectors_ = alwaysShowAllSelectors;
    pollTimer_.start(1000,this);
}

const char *LH_MonitoringSource::userInit()
{
    LH_QtMonitoringPlugin* plugin = dynamic_cast<LH_QtMonitoringPlugin*>(parent());
    setup_enabled_ = plugin->createUI_Element_Enabled(appName());
    setup_rate_ = plugin->createUI_Element_Rate(appName());
    plugin->createUI_Element_Divider(appName());

    clearSensors();
    afterUpdate();
    needsRebuild_ = true;
    pollTimer_.start( 0, this );
    return 0;
}

// clearSensors: remove all the existing sensor values
void LH_MonitoringSource::clearSensors()
{
    QString key;
    foreach (SensorType type, sensors_)
    {
        foreach(SensorGroup group, type.groups)
        {
            key = getLinkPath(type.name, group.name, "__array_ptr");
            if(setupItems_.contains(key))
                updateValueItem(key, 0);

            foreach(SensorItem item, group.items)
            {
                key = getLinkPath(type.name, group.name, item.name);
                if(setupItems_.contains(key))
                    updateValueItem(key, "N/A");
                if(setupItems_.contains(key+"/String"))
                    updateValueItem(key+"/String", "N/A");

                item.clear();
            }
            group.items.clear();
        }
        type.groups.clear();
    }
    sensors_.clear();
}

// updateValue: Force the existance of the type.group.item, then update the value via updateValueItem
void LH_MonitoringSource::updateValue(QString type, QString group, QString item, QVariant val)
{
    updateValue(type, group, item, val, SensorDefinition(), false, false);
}
void LH_MonitoringSource::updateValue(QString type, QString group, QString item, QVariant val, SensorDefinition def)
{
    updateValue(type, group, item, val, def, false, false);
}
void LH_MonitoringSource::updateValue(QString type, QString group, QString item, QVariant val, SensorDefinition def, bool isAggregate, bool isGroup)
{
    bool isNumeric;
    qreal fltVal = val.toFloat(&isNumeric);
    if(def.deadValue.exists && isNumeric && def.deadValue.value == fltVal)
        return; //dead!
    else
    {

         if(sensors_.contains("")) // Fix for mysterious blank item that sometimes appears in the list.
            sensors_.remove("");

        // add sensor type, if required
        if(!sensors_.contains(type))
        {
            SensorType sensorType;
            sensorType.name = type;
            sensors_.insert(type, sensorType);
        }

        // add sensor group, if required
        if(!sensors_[type].groups.contains(group))
        {
            SensorGroup sensorGroup;
            sensorGroup.name = group;
            sensorGroup.limits = def.limits;
            sensors_[type].groups.insert(group, sensorGroup);

            QString key = getLinkPath(type, group, "__array_ptr");
            updateValueItem(key, QVariant((int)&(sensors_[type].groups[group].items)), QMetaType::Int, "private/pointer");
        }

        // add sensor item, if required
        if(!sensors_[type].groups[group].items.contains(item))
        {
            //todo: move limits back onto the item and not the group, remove this assert and add it to updateAggregates instead
            Q_ASSERT_X(sensors_[type].groups[group].limits == def.limits, "LH_MonitoringSource::updateValue", "Inconsistent Limits");

            SensorItem sensorItem;
            sensorItem.name = item;
            sensorItem.units = def.units;
            sensorItem.adaptiveUnitsList.clear();
            if(def.adaptiveUnits.active)
            {
                sensorItem.adaptiveUnitsList.append(def.adaptiveUnits.list);
                sensorItem.adaptiveUnitsFactor = def.adaptiveUnits.factor;
            }
            else
                sensorItem.adaptiveUnitsFactor = 1;

            sensorItem.aggregate = isAggregate;
            sensorItem.group = isGroup;
            sensors_[type].groups[group].items.append(sensorItem);
        }

        // update the historic data
        bool isNumericValue = false;
        qreal numericValue = val.toReal(&isNumericValue);
        if(isNumericValue)
            sensors_[type].groups[group].items[item].addValue(numericValue, pollingRate());

        // now the sensor type.group.item exists, update the value

        QString key = getLinkPath(type, group, item);
        updateValueItem(key, val);
        updateValueItem(key+"/String", val, QMetaType::QString);
    }
}

// updateValueItem (private): creates or updates a setup item for holding the values
void LH_MonitoringSource::updateValueItem(QString key, QVariant val, QMetaType::Type valueType, const char *mimeType)
{
    if(valueType==QMetaType::Void)
    {
        if(setupTypes_.contains(key))
            valueType = getValueType(key);
        else
            valueType = (QMetaType::Type)val.type();
    }

    if(setupItems_.contains(key))
    {
        switch(valueType)
        {
        case QMetaType::Float:
            reinterpret_cast<LH_Qt_float*>(setupItems_.value(key))->setValue(val.toFloat());
            break;
        case QMetaType::Bool:
            reinterpret_cast<LH_Qt_bool*>(setupItems_.value(key))->setValue(val.toBool());
            break;
        case QMetaType::Int:
        case QMetaType::UInt:
            reinterpret_cast<LH_Qt_int*>(setupItems_.value(key))->setValue(val.toInt());
            break;
        case QMetaType::QString:
        case QMetaType::LongLong:
        case QMetaType::Double:
            reinterpret_cast<LH_Qt_QString*>(setupItems_.value(key))->setValue(val.toString());
            break;
        default:
            Q_ASSERT_X(false,"LH_MonitoringSource::updateValueItem","Unhandled type");
        }
    }
    else
    {
        LH_QtSetupItem* si = NULL;
        LH_QtObject* obj = this->parent();

        QString fieldName = QString("SourceFields/%1").arg(QString(key).replace("/"+QString(STRINGIZE(MONITORING_FOLDER)),""));
        while(fieldName.contains("//"))
            fieldName = fieldName.replace("//","/");

        switch(valueType)
        {
        case QMetaType::Float:
            si = new LH_Qt_float(obj, fieldName, val.toFloat(),
                                 std::numeric_limits<qreal>::min(), std::numeric_limits<qreal>::max(),
                                 LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSINK | LH_FLAG_HIDDEN_UNLESS_DEBUGGING);
            break;
        case QMetaType::Bool:
            si = new LH_Qt_bool(obj, fieldName, val.toBool(),
                                 LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSINK | LH_FLAG_HIDDEN_UNLESS_DEBUGGING);
            break;
        case QMetaType::Int:
        case QMetaType::UInt:
            si = new LH_Qt_int(obj, fieldName, val.toInt(),
                               std::numeric_limits<int>::min(), std::numeric_limits<int>::max(),
                               LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSINK | LH_FLAG_HIDDEN_UNLESS_DEBUGGING);
            break;
        case QMetaType::QString:
        case QMetaType::LongLong:
        case QMetaType::Double:
            si = new LH_Qt_QString(obj, fieldName, val.toString(),
                                   LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSINK | LH_FLAG_HIDDEN_UNLESS_DEBUGGING);
            break;
        default:
            Q_ASSERT_X(false,"LH_MonitoringSource::updateValueItem","Unhandled type");
        }
        si->setPublishPath(key);
        if(mimeType)
            si->setMimeType(mimeType);
        setupItems_.insert(key, si);
        setupTypes_.insert(key, valueType);

        needsRebuild_ = true;
    }
}

// getValueItem (private): used by updateAggregates; reads the value of a sensor via its sensorItem_
QVariant LH_MonitoringSource::getValueItem(QString key)
{
    if(!setupItems_.contains(key))
        return QVariant::fromValue(0);
    switch(setupItems_.value(key)->type())
    {
    case lh_type_integer:
        return QVariant::fromValue(reinterpret_cast<LH_Qt_int*>(setupItems_.value(key))->value());
    case lh_type_fraction:
        return QVariant::fromValue(reinterpret_cast<LH_Qt_float*>(setupItems_.value(key))->value());
    case lh_type_string:
        return QVariant::fromValue(reinterpret_cast<LH_Qt_QString*>(setupItems_.value(key))->value());
    default:
        Q_ASSERT_X(false,"LH_MonitoringSource::getValueItem","Unhandled type");
        return QVariant::fromValue(0);
    }
}

// updateAggregates: builds the aggregate items for a group: "min", "max", "avg" and "all" (
void LH_MonitoringSource::updateAggregates(QString type, QString group)
{
    if(!sensors_.contains(type))
        return;
    SensorType sensorType = sensors_[type];

    if(!sensorType.groups.contains(group))
        return;
    SensorGroup sensorGroup = sensorType.groups[group];

    DataLineCollection  sensorItems  = sensorGroup.items;

    if(sensorItems.count()!=0)
    {
        QString units = sensorItems[0].units;
        foreach(SensorItem item, sensorItems)
            Q_ASSERT_X(units == item.units, "LH_MonitoringSource::updateAggregates", QString("Inconsistent Units: [%1] vs. [%2]").arg(units).arg(item.units).toLocal8Bit() );
    }

    int count = 0;
    qreal avg = 0;
    qreal min = 0;
    qreal max = 0;

    bool ok;

    SensorDefinition def;

    QVariantMap jobject;

    // calculate aggregates
    foreach(SensorItem item, sensorItems)
        if(!item.aggregate)
        {
            QVariant val = getValueItem(getLinkPath(type, group, item.name));
            QVariantMap jsensor;
            jsensor.insert("value",val);
            jsensor.insert("units",item.units);
            jobject.insert(item.name,jsensor);
            qreal fltVal = val.toFloat(&ok);
            if(ok)
            {
                count++;
                avg += fltVal;
                if(count==1)
                {
                    def.units = item.units;
                    def.limits = sensorGroup.limits;
                }
                min = (count==1? fltVal : qMin(min, fltVal));
                max = (count==1? fltVal : qMax(max, fltVal));
            }
        }

    // add/update aggregates and groups
    if(count>1)
    {
        QString commonName = (group == ""? type : group);
        QString jsonData = QString::fromUtf8(Json::serialize(jobject));
        avg = qRound((avg*10.0) / count)/10.0;
        updateValue(type, group, QString("Average %1").arg(commonName), avg     , def, true, false);
        updateValue(type, group, QString("Minimum %1").arg(commonName), min     , def, true, false);
        updateValue(type, group, QString("Maximum %1").arg(commonName), max     , def, true, false);
        updateValue(type, group, "All"                                , jsonData, def, true, true);
    }
}

// reverse: string function to reverse a string, e.g. reverse("String") == "gnirtS"
QString LH_MonitoringSource::reverse(QString str)
{
    QByteArray ba = str.toLatin1();
    char *d = ba.data();
    std::reverse(d, d+str.length());
    return QString(d);
}

// getTypes: gets a list of all the available sensor types
QStringList LH_MonitoringSource::getTypes()
{
    QStringList l;
    foreach(SensorType sensorType, sensors_)
        l.append(sensorType.name);
    l.sort();
    return l;
}

// getType: returns the named sensor type
SensorType LH_MonitoringSource::getType(QString name)
{
    return sensors_.value(name);
}

// getGroups: gets a list of all the sensor groups listed within the specified type
QStringList LH_MonitoringSource::getGroups(QString type)
{
    QStringList l;
    if(sensors_.contains(type))
    {
        SensorType sensorType = sensors_[type];
        foreach(SensorGroup sensorGroup, sensorType.groups)
            l.append(sensorGroup.name);
        l.sort();
    }
    return l;
}

// getGroup: returns the named sensor group
SensorGroup* LH_MonitoringSource::getGroup(QString type, QString group, bool *ok)
{
    if(type!="" || group != "")
        if(sensors_.contains(type))
            if(sensors_[type].groups.contains(group))
            {
                if(ok) *ok = true;
                return &sensors_[type].groups[group];
            }
    if(ok) *ok = false;
    return NULL;
}

// getItems: gets a list of all the sensor items listed within the specified group
QStringList LH_MonitoringSource::getItems(QString type, QString group, bool includeGroups)
{
    QStringList itmList;

    if(sensors_.contains(type) && sensors_[type].groups.contains(group))
    {
        SensorGroup sensorGroup = sensors_[type].groups[group];

        foreach(SensorItem sensorItem, sensorGroup.items)
            if(!sensorItem.aggregate)
                itmList.append(sensorItem.name);
        itmList.sort();

        QStringList aggList;
        foreach(SensorItem sensorItem, sensorGroup.items)
            if(sensorItem.aggregate && (!sensorItem.group || includeGroups))
                aggList.append(sensorItem.name);
        aggList.sort();

        for(int i = 0; i<aggList.count(); i++)
            itmList.append(aggList[i]);
    }

    return itmList;
}

// getItem: returns the named sensor item
SensorItem LH_MonitoringSource::getItem(QString type, QString group, QString item, bool *ok, int *selectedIndex)
{
    if(type!="" || group != "" || item != "")
        if(sensors_.contains(type))
            if(sensors_[type].groups.contains(group))
                if(sensors_[type].groups[group].items.contains(item))
                {
                    if(ok) *ok = true;
                    if(selectedIndex)
                        *selectedIndex = sensors_[type].groups[group].items.indexOf(item);
                    return sensors_[type].groups[group].items[item];
                }
    if(ok) *ok = false;
    SensorItem dummy;
    return dummy;
}

// getValueType: returns the data type for the requested link path (e.g. string, int, qreal)
QMetaType::Type LH_MonitoringSource::getValueType(QString key)
{
    if(setupTypes_.contains(key))
        return setupTypes_[key];
    else
        return QMetaType::Void;
}

// getMonitoringOptions: compiles a map of sensor names (type.group.item) suitable for json conversion.
QVariantMap LH_MonitoringSource::getMonitoringOptions()
{
    QVariantMap types;

    foreach(SensorType type, sensors_)
    {
        QVariantMap groups;
        foreach(SensorGroup group, type.groups)
        {
            QVariantList items;
            foreach(SensorItem item, group.items)
                items.append(item.name);
            groups.insert(group.name, items);
        }
        types.insert(type.name, groups);
    }

    return types;
}

// afterUpdate: actions performed after an update is complete
void LH_MonitoringSource::afterUpdate()
{
    QString currentOptionsData = QString::fromUtf8(Json::serialize(getMonitoringOptions()));
    if (optionsData_ != currentOptionsData)
    {
        optionsData_ = currentOptionsData;
        getDataSources()->refreshMonitoringOptions();
    }
    updateValueItem(getLinkPath("","","")+"/dataAvailable", this->dataAvailable(), QMetaType::Bool, "private/bool");
}
