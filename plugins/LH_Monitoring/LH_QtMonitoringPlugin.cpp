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

#include "LH_QtMonitoringPlugin.h"

const char *LH_QtMonitoringPlugin::userInit()
{
    dataSources()->userInit();
    QTimer::singleShot(5000, this, SLOT(connectEnabledStateChangeEvents()));
    return 0;
}

LH_Qt_bool* LH_QtMonitoringPlugin::createUI_Element_Enabled(QString appName)
{
    enabled_.append( new LH_Qt_bool(this, "^"+appName, true, LH_FLAG_NOSOURCE | LH_FLAG_NOSINK | LH_FLAG_HIDETITLE ) );
    return enabled_.last();
}

LH_Qt_QSlider* LH_QtMonitoringPlugin::createUI_Element_Rate(QString appName)
{
    rate_helpers_.append( new LH_Qt_QString(this, appName + " update rate helper", "1 update per second", LH_FLAG_NOSOURCE | LH_FLAG_NOSINK | LH_FLAG_BLANKTITLE | LH_FLAG_READONLY | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA | LH_FLAG_HIDDEN) );
    rates_.append( new LH_Qt_QSlider(this, appName + " update rate", 0 , -4, 4, LH_FLAG_NOSOURCE | LH_FLAG_NOSINK | LH_FLAG_BLANKTITLE | LH_FLAG_HIDDEN ) );
    connect(rates_.last(), SIGNAL(changed()), this, SLOT(enabledFreqChanged()));
    return rates_.last();
}

LH_Qt_QString* LH_QtMonitoringPlugin::createUI_Element_Divider(QString appName)
{
    if(dividers_.length()!=0)
        dividers_.last()->setHidden(false);
    dividers_.append( new LH_Qt_QString(this, "~divider_"+appName, "<hr />", LH_FLAG_NOSOURCE | LH_FLAG_NOSINK | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA | LH_FLAG_HIDETITLE | LH_FLAG_HIDDEN, lh_type_string_html ) );
    return dividers_.last();
}

void LH_QtMonitoringPlugin::enabledStateChanged()
{
    for(int i=0; i<enabled_.length(); i++)
    {
        rate_helpers_[i]->setVisible(enabled_[i]->value());
        rates_[i]->setVisible(enabled_[i]->value());
    }
    dataSources()->refreshMonitoringOptions();
}

void LH_QtMonitoringPlugin::enabledFreqChanged()
{
    for(int i=0; i<rates_.length(); i++)
    {
        int val = rates_[i]->value();
        if(val < 0)
            rate_helpers_[i]->setValue(QString("%1 seconds per update").arg(1-val));
        if(val == 0)
            rate_helpers_[i]->setValue("1 update per second");
        if(val > 0)
            rate_helpers_[i]->setValue(QString("%1 updates per second").arg(val+1));
    }
}

void LH_QtMonitoringPlugin::connectEnabledStateChangeEvents()
{
    qDebug() << "LH_QtMonitoringPlugin events connected";
    foreach(LH_Qt_bool *b, enabled_)
        connect(b, SIGNAL(changed()), this, SLOT(enabledStateChanged()));
    enabledStateChanged();
}
