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

#ifndef LH_MONITORINGSOURCES_H
#define LH_MONITORINGSOURCES_H

#include <QString>
#include <QStringList>
#include <QHash>
#include <QTimer>

#include "LH_MonitoringSource.h"
#include "json.h"

class LH_MonitoringSources: public LH_QtObject
{
    Q_OBJECT

    QHash<QString, LH_MonitoringSource*> sources_;
    QTimer* rebuildTimer_;

    LH_Qt_QString* setup_monitoring_options_;
public:
    LH_MonitoringSources(LH_QtObject *parent);

    virtual const char *userInit();
    virtual void userTerm();

    bool needsRebuild();
    void clearNeedsRebuild();
    bool rebuild();

    QStringList getApplications();

    LH_MonitoringSource* source(QString key) { return (sources_.contains(key)? sources_.value(key) : NULL); }

    bool add(LH_MonitoringSource* source);

    QString getLinkPath(QString app,QString type,QString group,QString item) { return (sources_.contains(app)?  sources_.value(app)->getLinkPath(type,group,item) : ""); }

    QString getOptionsLinkPath() { return QString("/%1/Options").arg(STRINGIZE(MONITORING_FOLDER)); }

    void refreshMonitoringOptions();

public slots:
    void doRebuild();

};

LH_MonitoringSources* getDataSources();

#endif // LH_MONITORINGSOURCES_H
