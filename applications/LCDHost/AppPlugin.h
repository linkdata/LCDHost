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

#ifndef APPPLUGIN_H
#define APPPLUGIN_H

#if 0

#include <QObject>
#include <QMutex>
#include <QImage>
#include <QFileInfo>
#include <QMetaType>
#include <QCoreApplication>

// This class represents a plugin at the highest level;
// it caches plugin data and interfaces with the UI

#include "LCDHost.h"
#include "AppLibrary.h"
#include "AppPluginThread.h"

class AppClass;

class AppPlugin : public AppLibrary
{
    Q_OBJECT

    QList<AppClass*> classlist_;
    void bindClass( AppClass *app_class );

public:
    AppPlugin( QFileInfo fi = QFileInfo() );
    ~AppPlugin();

    bool event(QEvent *);

    AppPluginThread *libThreadCreate() { return new AppPluginThread(this,fileInfo()); }

    static AppPlugin *fromId(AppId id) { return static_cast<AppPlugin*>(AppLibrary::fromId(id)); }

public slots:
    void layoutChanged( QString dir, QString name );
};
#endif

#endif // APPPLUGIN_H
