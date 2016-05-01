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

#ifndef APPVERSIONCACHE_H
#define APPVERSIONCACHE_H

#include <QObject>
#include <QUrl>
#include <QMap>
#include <QTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "AppPluginVersion.h"

class AppVersionCache : public QObject
{
    Q_OBJECT

    QList<AppPluginVersion> files_;
    QMap<QUrl,QTime> sources_;
    int refresh_; // source refresh in seconds

    void addSource( QUrl url );

public:
    explicit AppVersionCache(QObject *parent = 0);

    void add( QString id, QUrl url, QString arch = AppPluginVersion::defaultArch() );
    const AppPluginVersion& get( QString id, QString arch = AppPluginVersion::defaultArch() ) const;

    QList<QUrl> staleUrls();
    void startedQuery(QUrl u) { if( sources_.contains(u) ) sources_[u].restart(); }
    bool wantsUrl(QUrl u) const { return sources_.contains(u); }
    QString parseReply( QNetworkReply *r );
};

#endif // APPVERSIONCACHE_H
