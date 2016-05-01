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

#ifndef PLUGININFO_H
#define PLUGININFO_H

#include <QObject>
#include <QFileInfo>
#include <QByteArray>
#include <QXmlStreamReader>

#include "lh_plugin.h"

class PluginInfo : public QObject
{
    Q_OBJECT

    QFileInfo fileinfo_;

    // plugin data
    int xmlindex_;          //< File offset where XML info was found
    QString id_;
    int revision_;
    int api_major_;
    int api_minor_;
    QString version_;
    QString versionurl_;
    QString author_;
    QString homepageurl_;
    QString logourl_;
    QString shortdesc_;
    QString longdesc_;
    QByteArray sha1_;       //< SHA1 of library

    // settings stored stuff
    bool enabled_;

    void clear();
    QString readXML(const QByteArray&);
    QString readBlock(const QByteArray& contents, qint64 xmlpos, QXmlStreamReader& stream);

public:
    explicit PluginInfo( QObject *parent = 0 );

    // These all return an empty string or an error message string
    // QString read( QFileInfo fi, QByteArray *save_contents = 0 );
    QString read() { return read(fileinfo_); }
    QString read(const QFileInfo& fi);

    // QString verifyURL() const { return sig_.size == sizeof(lh_signature) ? sig_.url : QString(); }
    bool isAPI5Plugin() const { return xmlindex_ >= 0; }
    bool isValid() const { return (xmlindex_ >= 0) && (!id_.isEmpty()); }
    bool isUsable() const { return isValid() && (api_major_ == LH_API_MAJOR) && (api_minor_<=LH_API_MINOR); }

    bool isEnabled() const { return enabled_; }
    bool isModified() const;

    void setEnabled( bool b );

    const QFileInfo& fileInfo() const { return fileinfo_; }

    QString id() const { return id_; }
    int revision() const { return revision_; }
    int api_major() const { return api_major_; }
    int api_minor() const { return api_minor_; }
    QString version() const { return version_; }
    QString versionurl() const { return versionurl_; }
    QString author() const { return author_; }
    QString homepageurl() const { return homepageurl_; }
    QString logourl() const { return logourl_; }
    QString shortdesc() const { return shortdesc_; }
    QString longdesc() const { return longdesc_; }

    PluginInfo& operator=( const PluginInfo& other )
    {
        if( this != &other )
        {
            fileinfo_ = other.fileinfo_;
            xmlindex_ = other.xmlindex_;
            id_ = other.id_;
            revision_ = other.revision_;
            api_major_ = other.api_major_;
            api_minor_ = other.api_minor_;
            version_ = other.version_;
            versionurl_ = other.versionurl_;
            author_ = other.author_;
            homepageurl_ = other.homepageurl_;
            logourl_ = other.logourl_;
            shortdesc_ = other.shortdesc_;
            longdesc_ = other.longdesc_;
            sha1_ = other.sha1_;
            enabled_ = other.enabled_;
        }
        return *this;
    }
};

#endif // PLUGININFO_H

