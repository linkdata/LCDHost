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

#ifndef APPSETUPLINK_H
#define APPSETUPLINK_H

#include <QObject>
#include <QList>

class AppSetupItem;

class AppSetupLink : public QObject
{
    Q_OBJECT

    explicit AppSetupLink(const QString &path);

public:
    static QObject* master();
    static AppSetupLink *factory(const QString &path);
    static QList<AppSetupLink *> list();
    
signals:
    void requestPublishing();
    void publishing(const AppSetupItem * asi);

public slots:
    void requestPublish();
    void publish(const AppSetupItem * asi);
};

#endif // APPSETUPLINK_H
