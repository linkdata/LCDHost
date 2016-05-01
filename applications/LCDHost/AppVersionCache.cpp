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


#include "AppVersionCache.h"
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QStringList>
#include <QDebug>

void AppPluginVersion::update( QString ut, int rev, QString api )
{
    QStringList parts = api.split('.');
    urltemplate_ = ut;
    rev_ = rev;
    api_major_ = parts.first().toInt();
    api_minor_ = parts.last().toInt();
}

AppVersionCache::AppVersionCache(QObject *parent) : QObject(parent)
{
    refresh_ = 60;
}


void AppVersionCache::addSource( QUrl url )
{
    if( !sources_.contains(url) )
    {
        sources_.insert( url, QTime() );
    }
}

void AppVersionCache::add( QString id, QUrl url, QString arch )
{
    for( int i=0; i<files_.size(); ++i )
    {
        if( files_[i].id() == id && files_[i].arch() == arch )
        {
            files_[i].setVersionURL( url );
            addSource( url );
            return;
        }
    }

    files_.append( AppPluginVersion(id,url,arch) );
    addSource( url );
}

const AppPluginVersion& AppVersionCache::get( QString id, QString arch ) const
{
    static AppPluginVersion blank_;
    for( int i=0; i<files_.size(); ++i )
    {
        if( files_[i].id() == id && files_[i].arch() == arch )
        {
            return files_[i];
        }
    }
    return blank_;
}

QList<QUrl> AppVersionCache::staleUrls()
{
    QList<QUrl> retv;
    for( QMap<QUrl,QTime>::const_iterator i = sources_.constBegin(); i != sources_.constEnd(); ++i )
    {
        QTime age = i.value();
        if( age.isNull() || age.elapsed() > (1000*refresh_) )
            retv.append( i.key() );
    }
    return retv;
}

QString AppVersionCache::parseReply( QNetworkReply *reply )
{
    QString default_arch;
    QString default_url;
    QString default_api;

    if( reply == NULL )
        return "NULL reply";

    if( !sources_.contains(reply->url()) )
        return QString("Source URL not handled: %1").arg(reply->url().toString());

    QXmlStreamReader stream(reply);

    while( !stream.atEnd() )
    {
        stream.readNext();

        if( stream.isEndElement() && stream.name() == "lhver" )
            break;

        if( stream.isStartElement() && stream.name() == "lhver" )
        {
            // extract defaults
            default_arch = stream.attributes().value("arch").toString();
            default_url = stream.attributes().value("url").toString();
            default_api = stream.attributes().value("api").toString();
        }

        if( stream.isStartElement() && stream.name() == "f" )
        {
            QString id = stream.attributes().value("id").toString();
            int revision = stream.attributes().value("r").toString().toInt();
            if( !id.isEmpty() && revision )
            {
                QString arch = default_arch;
                QString urltemplate = default_url;
                QString api = default_api;

                if( stream.attributes().hasAttribute("arch") )
                    arch = stream.attributes().value("arch").toString();
                if( stream.attributes().hasAttribute("url") )
                    urltemplate = stream.attributes().value("url").toString();
                if( stream.attributes().hasAttribute("api") )
                    api = stream.attributes().value("api").toString();

                if( !arch.isEmpty() )
                {
                    int i;
                    for( i=0; i<files_.size(); ++i )
                    {
                        if( files_[i].id() == id && files_[i].arch() == arch )
                            break;
                    }
                    if( i >= files_.size() )
                    {
                        files_.append( AppPluginVersion(id,reply->url(),arch) );
                        Q_ASSERT( i == files_.size()-1 );
                        Q_ASSERT( files_[i].id() == id );
                    }
                    files_[i].update( urltemplate, revision, api );
                }
            }
        }
    }

    sources_[reply->url()].start();

    return QString();
}

