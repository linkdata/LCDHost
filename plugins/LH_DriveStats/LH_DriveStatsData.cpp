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

#include "LH_DriveStatsData.h"

#include <QDebug>
#include <QRegExp>
#include <algorithm>
#include <QSettings>

LH_DriveStatsData::LH_DriveStatsData(LH_QtObject *parent, LH_MonitoringUI *ui, monitoringDataMode dataMode, bool includeGroups) : LH_MonitoringData( parent, ui, dataMode, includeGroups, true)
{
    Q_UNUSED(includeGroups);

    ui_->reset();
    ui_->setVisible(mon_item | mon_type);

    if(dataMode & mdmNumbers) ui_->append(mon_item, "Bytes Read");
    if(dataMode & mdmNumbers) ui_->append(mon_item, "Bytes Written");
    if(dataMode & mdmNumbers) ui_->append(mon_item, "Read Count");
    if(dataMode & mdmNumbers) ui_->append(mon_item, "Write Count");
    if(dataMode & mdmNumbers) ui_->append(mon_item, "Read Time");
    if(dataMode & mdmNumbers) ui_->append(mon_item, "Write Time");
    if(dataMode & mdmNumbers) ui_->append(mon_item, "Idle Time");
    if(dataMode & mdmNumbers) ui_->append(mon_item, "Queue Depth");
    if(dataMode & mdmNumbers) ui_->append(mon_item, "Free Space");
    if(dataMode & mdmNumbers) ui_->append(mon_item, "Used Space");
    if(dataMode & mdmText) ui_->append(mon_item, "Total Space");

    if(dataMode & mdmPie) ui_->append(mon_item, "Disk Space");

    QStringList drives = driveInfo.getDrives();
    for(int i=0; i<drives.count(); i++)
        ui_->append(mon_type, drives.at(i) );

    driveInfo.setDrive(ui_->valueText(mon_type));

    ui_->refresh(mon_item | mon_type);
}

bool LH_DriveStatsData::getAdaptiveUnitOptions(unitOptionsType &options)
{
    QString item = ui_->valueText(mon_item);
    if(item == "Bytes Read" || item == "Bytes Written" || item == "Free Space" ||  item == "Used Space" || item == "Total Space")
    {
        options.set(1024, new QStringList(QStringList()<<"B"<<"kB"<<"MB"<<"GB"<<"TB"));
        return true;
    }
    else
        return false;
}

bool LH_DriveStatsData::getData(float& value, QString& text, QString& units)
{

    value = 0;
    text = "";
    units = "";

    if(driveInfo.setDrive(ui_->valueText(mon_type)))
    {
        if(dataMode_ & mdmNumbers) ui_->append(mon_item, "Bytes Read");
        if(dataMode_ & mdmNumbers) ui_->append(mon_item, "Bytes Written");
        if(dataMode_ & mdmNumbers) ui_->append(mon_item, "Read Count");
        if(dataMode_ & mdmNumbers) ui_->append(mon_item, "Write Count");
        if(dataMode_ & mdmNumbers) ui_->append(mon_item, "Read Time");
        if(dataMode_ & mdmNumbers) ui_->append(mon_item, "Write Time");
        if(dataMode_ & mdmNumbers) ui_->append(mon_item, "Idle Time");
        if(dataMode_ & mdmNumbers) ui_->append(mon_item, "Queue Depth");
        if(dataMode_ & mdmNumbers) ui_->append(mon_item, "Free Space");
        if(dataMode_ & mdmNumbers) ui_->append(mon_item, "Used Space");
        if(dataMode_ & mdmNumbers) ui_->append(mon_item, "Total Space");

        if(dataMode_ & mdmPie) ui_->append(mon_item, "Disk Space");

        ui_->refresh(mon_item);
        ui_->changeItemSelection();
    }
    else
        driveInfo.update();
    if(driveInfo.valid())
    {
        units = (this->adaptiveUnits() && ui_->setup_unit_selection_->valueText()=="(Auto)")? "" : ui_->setup_unit_selection_->valueText();

        if(ui_->valueText(mon_item) == "Bytes Read")
            parseBytes(driveInfo.BytesRead(), value, text, units);
        if(ui_->valueText(mon_item) == "Bytes Written")
            parseBytes(driveInfo.BytesWritten(), value, text, units);
        if(ui_->valueText(mon_item) == "Read Count")
            value = driveInfo.ReadCount();
        if(ui_->valueText(mon_item) == "Write Count")
            value = driveInfo.WriteCount();
        if(ui_->valueText(mon_item) == "Read Time")
        {
            value = driveInfo.ReadTime();
            units = "%";
        }
        if(ui_->valueText(mon_item) == "Write Time")
        {
            value = driveInfo.WriteTime();
            units = "%";
        }
        if(ui_->valueText(mon_item) == "Idle Time")
        {
            value = driveInfo.IdleTime();
            units = "%";
        }
        if(ui_->valueText(mon_item) == "Queue Depth")
            value = driveInfo.QueueDepth();
        if(ui_->valueText(mon_item) == "Free Space")
            parseBytes(driveInfo.FreeSpace(), value, text, units);
        if(ui_->valueText(mon_item) == "Used Space")
            parseBytes(driveInfo.UsedSpace(), value, text, units);
        if(ui_->valueText(mon_item) == "Total Space")
            parseBytes(driveInfo.TotalSpace(), value, text, units);

        if(ui_->valueText(mon_item) == "Disk Space")
            value = driveInfo.UsedSpace();
    }

    return driveInfo.valid();
}

bool LH_DriveStatsData::getUpperLimit(float &value)
{
    if(ui_->valueText(mon_item) == "Free Space" || ui_->valueText(mon_item) == "Used Space" || ui_->valueText(mon_item) == "Disk Space")
    {
        value = driveInfo.TotalSpace();
        unitOptionsType options;
        if(getAdaptiveUnitOptions(options))
            options.convert(value, this->adaptiveUnits()? "" : ui_->setup_unit_selection_->valueText());

        return driveInfo.valid();
    }
    else
        return false;
}


void LH_DriveStatsData::parseBytes(qlonglong bytes, float& value, QString& text, QString& units)
{
    value = bytes;
    unitOptionsType options;
    if(getAdaptiveUnitOptions(options))
        units = options.convert(value, units);
    text = QString::number(value, 'f', 0);
}

monitoringDataType LH_DriveStatsData::getType()
{
    if(ui_->valueText(mon_item).endsWith("Time"))
        return mdtFloat;
    return mdtInt;
}

