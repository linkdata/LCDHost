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

#ifndef APPPLUGINVERSION_H
#define APPPLUGINVERSION_H

#include <QString>
#include <QUrl>

class AppPluginVersion
{
    QString id_;
    QUrl versionurl_;
    QString arch_;
    QString urltemplate_;
    int rev_;
    int api_major_;
    int api_minor_;

public:
    AppPluginVersion(const AppPluginVersion& other)
        : id_(other.id_), versionurl_(other.versionurl_), arch_(other.arch_),
          urltemplate_(other.urltemplate_), rev_(other.rev_),
          api_major_(other.api_major_), api_minor_(other.api_minor_) {}
    AppPluginVersion() { rev_ = api_major_ = api_minor_ = 0; }
    AppPluginVersion( QString id, QUrl versionurl, QString arch = defaultArch() )
        : id_(id), versionurl_(versionurl), arch_(arch),
          urltemplate_(QString()), rev_(0), api_major_(0), api_minor_(0) {}

    QString id() const { return id_; }
    QString arch() const { return arch_; }
    QUrl versionurl() const { return versionurl_; }
    QString urltemplate() const { return urltemplate_; }
    int revision() const { return rev_; }
    int api_major() const { return api_major_; }
    int api_minor() const { return api_minor_; }
    QUrl downloadurl() const { return makeUrl(urltemplate_,id_,rev_,arch_); }

    void setVersionURL( QUrl url ) { versionurl_ = url; }

    void update( QString ut, int rev, QString api );

    bool isValid() const { return (!id_.isEmpty()) && rev_>0 && api_major_>0; }
    bool isUsable() const;

    bool operator==( const AppPluginVersion& other ) const { return (id_ == other.id_) && (arch_==other.arch_); }
    const AppPluginVersion& operator=(const AppPluginVersion& other);

    static QString defaultArch();
    static QUrl makeUrl( QString templ, QString id = QString(), int rev = 0, QString arch = defaultArch() );
};


#endif // APPPLUGINVERSION_H
