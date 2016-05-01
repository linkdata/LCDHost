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

#include "AppSetupLink.h"
#include "AppObject.h"
#include "AppState.h"

QObject* AppSetupLink::master()
{
    static QObject* master_ = new QObject();
    return master_;
}

AppSetupLink *AppSetupLink::factory(const QString &path)
{
    if(path.isEmpty()) return 0;
    AppSetupLink *link = master()->findChild<AppSetupLink *>(path);
    return link ? link : new AppSetupLink(path);
}

QList<AppSetupLink *> AppSetupLink::list()
{
    return master()->findChildren<AppSetupLink *>();
}

AppSetupLink::AppSetupLink(const QString &path) :
    QObject(master())
{
    setObjectName(path);
}

void AppSetupLink::requestPublish()
{
    emit requestPublishing();
}

void AppSetupLink::publish(const AppSetupItem * asi)
{
    emit publishing(asi);
}
