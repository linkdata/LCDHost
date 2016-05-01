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

#ifndef LH_MONITORINGSOURCE_H
#define LH_MONITORINGSOURCE_H

#include <QObject>
#include <QDebug>
#include <QBasicTimer>
#include <QVariant>
#include <QList>
#include <QTimerEvent>
#include <QStringList>
#include "LH_QtObject.h"

#include "LH_Qt_bool.h"
#include "LH_Qt_float.h"
#include "LH_Qt_int.h"
#include "LH_Qt_QString.h"
#include "LH_Qt_QSlider.h"

#include "DataCollection.h"
#include "json.h"

typedef struct {bool exists; qreal value;} OptionalValue;
inline bool operator==(OptionalValue v1, OptionalValue v2) { return v1.exists == v2.exists && v1.value == v2.value; }
typedef struct {OptionalValue minimum; OptionalValue maximum;} minmax;
inline bool operator==(minmax v1, minmax v2) { return v1.minimum == v2.minimum && v1.maximum == v2.maximum; }

typedef DataLine SensorItem ;

typedef struct {QString name; DataLineCollection items; minmax limits;} SensorGroup;
typedef QHash<QString, SensorGroup> SensorGroups;
typedef struct {QString name; SensorGroups groups;} SensorType;
typedef QHash<QString, SensorType> SensorTypes;

typedef struct {bool active; int factor; QStringList list;} AdaptiveUnits;

class SensorDefinition
{

    void init()
    {
        OptionalValue NA_ = { false, 0 };
        minmax _def_limits = { NA_, NA_ };
        this->units = "";
        this->limits = _def_limits;
        this->deadValue = NA_;
        adaptiveUnits.active = false;
        adaptiveUnits.factor = 1;
    }

public:
    QString units;
    minmax limits;
    OptionalValue deadValue;
    AdaptiveUnits adaptiveUnits;

    SensorDefinition()
    {
        init();
    }

    SensorDefinition(QString units)
    {
        init();
        this->units = units;
    }

    SensorDefinition(QString units, qreal minimum_limit, qreal maximum_limit)
    {
        OptionalValue _def_min = { true, minimum_limit };
        OptionalValue _def_max = { true, maximum_limit };
        init();
        this->units = units;
        this->limits.minimum = _def_min;
        this->limits.maximum = _def_max;
    }

    SensorDefinition(QString units, qreal minimum_limit, qreal maximum_limit, qreal deadValue)
    {
        OptionalValue _def_min = { true, minimum_limit };
        OptionalValue _def_max = { true, maximum_limit };
        OptionalValue _def_dead = { true, deadValue };
        init();
        this->units = units;
        this->limits.minimum = _def_min;
        this->limits.maximum = _def_max;
        this->deadValue = _def_dead;
    }

    SensorDefinition(QString units, qreal deadValue)
    {
        OptionalValue _def_dead = { true, deadValue };
        init();
        this->units = units;
        this->deadValue = _def_dead;
    }

    void setAdaptiveUnits(int factor, QStringList unitsList)
    {
        adaptiveUnits.active = true;
        adaptiveUnits.factor = factor;
        adaptiveUnits.list.clear();
        adaptiveUnits.list.append(unitsList);
    }

};

class LH_MonitoringSource : public LH_QtObject
{
    Q_OBJECT

    QBasicTimer pollTimer_;
    QHash<QString, LH_QtSetupItem*> setupItems_;
    QHash<QString, QMetaType::Type> setupTypes_;
    bool needsRebuild_;

    void updateValueItem(QString key, QVariant val, QMetaType::Type valueType = QMetaType::Void, const char *mimeType = NULL);
    QVariant getValueItem(QString key);

    QString optionsData_;
    bool dataAvailable_;
    bool alwaysShowAllSelectors_;
    QString appName_;

protected:
    LH_Qt_bool* setup_enabled_;
    LH_Qt_QSlider* setup_rate_;

    void updateAggregates(QString type, QString group);

    void setDataAvailable(bool dataAvailable) {
        if(dataAvailable_ != dataAvailable)
        {
            dataAvailable_ = dataAvailable;
            clearSensors();
            emit availabiltyChanged();
        }
    }

    SensorTypes sensors_;

    QString reverse(QString str);
    void clearSensors();
    void updateValue(QString type, QString group, QString item, QVariant val);
    void updateValue(QString type, QString group, QString item, QVariant val, SensorDefinition def);
    void updateValue(QString type, QString group, QString item, QVariant val, SensorDefinition def, bool isAggregate, bool isGroup);
    void timerEvent(QTimerEvent *event)
    {
        if (event->timerId() != pollTimer_.timerId())
            LH_QtObject::timerEvent(event);
        else
        {
            if(int t = polling())
                pollTimer_.start( t, this );
            else
                pollTimer_.stop();
        }
    }

    virtual void beforeUpdate() { return;}
    virtual bool doUpdate() { Q_ASSERT_X(false, "LH_MonitoringSource::update", "Update procedure not defined!"); return false;}
    virtual void afterUpdate();

    int pollingRate()
    {
        int val = setup_rate_->value();
        int ms = 1000;
        if(val < 0)
            ms = 1000 * (1-val);
        if(val > 0)
            ms = qRound(1000.0 / (val+1));
        return ms;
    }

public:
    QString getLinkPath(QString type, QString group, QString item)
    {
        return QString("/%1/%2/%3/%4/%5").arg(STRINGIZE(MONITORING_FOLDER)).arg(appName()).arg(type).arg(group).arg(item).replace("//","/");
    }

    LH_MonitoringSource(LH_QtObject *parent, QString appName, bool alwaysShowAllSelectors = false);

    bool enabled() { return (setup_enabled_? setup_enabled_->value() : false); }

    bool alwaysShowAllSelectors(){return alwaysShowAllSelectors_;}
    bool dataAvailable() { return dataAvailable_; }

    virtual const char *userInit();

    virtual void userTerm() { return; }

    bool update() {
        beforeUpdate();
        bool result = doUpdate();
        if(result)
            afterUpdate();
        return result;
    }

    QString appName() { return appName_; }

    virtual int polling() {
        if(setup_enabled_ && (setup_enabled_)->value())
        {
            //qDebug() << "update - " << this->appName() << (setup_enabled_)->value() << " : next in " << pollingRate() << "ms";
            update();
            return pollingRate();
        }
        else
            return 2000;
    }

    bool needsRebuild() { return needsRebuild_; }
    void clearNeedsRebuild() { needsRebuild_ = false; }

    QStringList getTypes();

    QStringList getGroups(QString type);

    SensorItem getItem(QString type, QString group, QString item, bool *ok=0, int *selectedIndex=0);
    SensorGroup* getGroup(QString type, QString group, bool *ok=0);


    QStringList getItems(QString type, QString group, bool includeGroups);

    SensorType getType(QString name);

    QMetaType::Type getValueType(QString key);

    QVariantMap getMonitoringOptions();

signals:
    void availabiltyChanged();
};

#endif // LH_MONITORINGSOURCE_H
