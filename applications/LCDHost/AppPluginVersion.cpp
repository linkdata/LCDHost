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

#include "AppPluginVersion.h"
#include "lh_plugin.h"

QString AppPluginVersion::defaultArch()
{
    static QString defaultarch;

    if( defaultarch.isEmpty() )
    {
#ifdef Q_OS_LINUX
        defaultarch = "lin";
#endif
#ifdef Q_OS_MAC
        defaultarch = "mac";
#endif
#ifdef Q_OS_WIN
        defaultarch = "win";
#endif
        defaultarch.append(QString::number(QSysInfo::WordSize));
#ifndef QT_NO_DEBUG
        defaultarch.append("d");
#endif
    }

    return defaultarch;
}

QUrl AppPluginVersion::makeUrl( QString templ, QString id, int rev, QString arch )
{
    templ.replace("https://","http://");
    templ.replace("$ID",id);
    templ.replace("$NAME",id);
    templ.replace("$ARCH",arch);
    templ.replace("$REV",QString::number(rev));
    return QUrl(templ);
}

const AppPluginVersion& AppPluginVersion::operator=(const AppPluginVersion& other)
{
    if( this != &other )
    {
        id_ = other.id_;
        versionurl_ = other.versionurl_;
        arch_ = other.arch_;
        urltemplate_ = other.urltemplate_;
        rev_ = other.rev_;
        api_major_ = other.api_major_;
        api_minor_ = other.api_minor_;
    }
    return *this;
}

bool AppPluginVersion::isUsable() const
{
    return isValid() && (api_major_ == LH_API_MAJOR) && (api_minor_<=LH_API_MINOR);
}
