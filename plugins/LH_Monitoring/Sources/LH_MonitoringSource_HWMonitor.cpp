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

#include "LH_MonitoringSource_HWMonitor.h"

#include <QSettings>
#include <QDebug>
//#include <algorithm>

LH_MonitoringSource_HWMonitor::LH_MonitoringSource_HWMonitor(LH_QtObject *parent): LH_MonitoringSource(parent, "HWMonitor (+ HWMonTray)") {}

bool LH_MonitoringSource_HWMonitor::doUpdate()
{
#ifndef Q_OS_WIN
    return false;
#else
    bool resultVal = false;

    QRegExp rx("([FTPV])\\[(.*)\\]");
    QRegExp rx2("(.*)-([0-9]*)");
    QRegExp rx3("(.*)#([0-9]*)");

    QList<QString> aggGroups;

    QSettings reg("HKEY_CURRENT_USER\\Software\\CPUID\\HWMonitor\\VistaSidebar", QSettings::NativeFormat);
    setDataAvailable(reg.childKeys().count()!=0);
    if(dataAvailable())
    {
        resultVal = true;

        QStringList keysList = reg.childKeys();
        keysList.sort();
        foreach(QString key, keysList)
        {
            if(rx.indexIn(key) == -1)
                continue;
            QString keyType = rx.cap(1);
            QString keyName = rx.cap(2);
            QString groupName = "";

            if(rx2.indexIn(keyName) != -1)
            {
                if(rx2.cap(2)=="0" && !reg.childKeys().contains(QString("%1[%2-1]").arg(keyType,rx2.cap(1))))
                {
                    keyName = rx2.cap(1);
                    if(rx3.indexIn(keyName) != -1)
                        groupName = rx3.cap(1);
                }
                else
                    keyName = QString("%1 (%2)").arg(rx2.cap(1), rx2.cap(2));
            }
            if(groupName=="")
                groupName = keyName;

            QString units = ""; QString type="Other";

            if(keyType=="F") {units="rpm"; type = "Fan Speed";}
            if(keyType=="T") {units=QLatin1Literal("\260C"); type = "Temperature";}
            if(keyType=="P") {units="W"; type = "Powers";}
            if(keyType=="V") {units="V"; type = "Voltage";}

            QVariant val = reg.value(key);

            if(!aggGroups.contains(QString("%1|~>|%2").arg(type).arg(groupName)))
                aggGroups.append(QString("%1|~>|%2").arg(type).arg(groupName));

            updateValue(type,groupName,keyName,val,SensorDefinition(units));
        }
        foreach(QString aggGroup, aggGroups)
        {
            QStringList aggItem = aggGroup.split("|~>|");
            updateAggregates(aggItem[0],aggItem[1]);
        }

    }
    return resultVal;
#endif
}
