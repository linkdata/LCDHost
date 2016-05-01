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


#include <QtDebug>
#include <QInputDialog>
#include <QCoreApplication>
#include <QProgressDialog>
#include <QXmlStreamReader>
#include <QCryptographicHash>
#include <QSettings>

#include "PluginInfo.h"

PluginInfo::PluginInfo( QObject *parent ) : QObject(parent)
{
    clear();
}

void PluginInfo::clear()
{
    sha1_.clear();
    id_.clear();
    xmlindex_ = -1;
    revision_ = -1;
    api_major_ = api_minor_ = 0;
    enabled_ = false;
}

QString PluginInfo::readBlock( const QByteArray& contents, qint64 xmlpos, QXmlStreamReader& stream )
{
    qint64 start = stream.characterOffset();
    stream.readElementText(QXmlStreamReader::IncludeChildElements);
    qint64 len = stream.characterOffset() - start - stream.name().length() - 3;
    if( len > 0 ) return QString::fromUtf8( contents.mid(xmlpos+start,len));
    return QString();
}

QString PluginInfo::readXML(const QByteArray &contents )
{
    const char *lcdhostpluginstarttag = "<lcdhostplugin>";
    const char *lcdhostpluginendtag = "</lcdhostplugin>";
    int starttagpos, endtagpos, xmlpos, xmllen;

    starttagpos = contents.indexOf(lcdhostpluginstarttag);
    if( starttagpos < 0 )
        return tr("no lcdhostplugin start tag");
    if( contents.indexOf(lcdhostpluginstarttag,starttagpos+1) >= 0 )
        return tr("multiple lcdhostplugin start tags");

    endtagpos = contents.indexOf(lcdhostpluginendtag);
    if( endtagpos < 0 )
        return tr("no lcdhostplugin end tag");
    if( contents.indexOf(lcdhostpluginendtag,endtagpos+1) >= 0 )
        return tr("multiple lcdhostplugin end tags");

    if( starttagpos > endtagpos )
        return tr("start and end lcdhostplugin tags reversed");

    xmlpos = contents.lastIndexOf("<?xml",starttagpos);
    if( xmlpos < 0 )
        return tr("no XML start tag");
    if( starttagpos - xmlpos > 256 )
        return tr("unresonably large XML start tag");

    xmllen = endtagpos - xmlpos + (int) qstrlen(lcdhostpluginendtag);

    QXmlStreamReader stream( contents.mid( xmlpos, xmllen ) );
    while( !stream.atEnd() )
    {
        stream.readNext();

        if( stream.isStartElement() )
        {
            if( stream.name() == "id" )
                id_ = stream.readElementText(QXmlStreamReader::IncludeChildElements);
            if( stream.name() == "rev" )
                revision_ = stream.readElementText(QXmlStreamReader::IncludeChildElements).toInt();
            if( stream.name() == "api" )
            {
                QStringList sl = stream.readElementText(QXmlStreamReader::IncludeChildElements).split('.');
                api_major_ = sl.first().toInt();
                api_minor_ = sl.last().toInt();
            }
            if( stream.name() == "ver" ) version_ = readBlock( contents, xmlpos, stream );
            if( stream.name() == "versionurl" ) versionurl_ = readBlock( contents, xmlpos, stream );
            if( stream.name() == "author" ) author_ = readBlock( contents, xmlpos, stream );
            if( stream.name() == "homepageurl" ) homepageurl_ = readBlock( contents, xmlpos, stream );
            if( stream.name() == "logourl" ) logourl_ = readBlock( contents, xmlpos, stream );
            if( stream.name() == "shortdesc" ) shortdesc_ = readBlock( contents, xmlpos, stream );
            if( stream.name() == "longdesc" ) longdesc_ = readBlock( contents, xmlpos, stream );
        }

        if( stream.hasError() )
        {
            qWarning() << tr("%1: XML error near '%2': %3")
                          .arg(fileinfo_.fileName())
                          .arg(QString::fromUtf8(contents.mid(xmlpos+stream.characterOffset(),20)).trimmed())
                          .arg(stream.errorString());
            break;
        }
    }

    if( id_.isEmpty() || revision_ < 0 )
        return tr("Can't parse XML for required tags 'id' and/or 'revision'");

    xmlindex_ = xmlpos;

    return QString();
}

QString PluginInfo::read(const QFileInfo& fi)
{
    clear();
    fileinfo_ = fi;
    if(!fileinfo_.exists())
        return tr("file not found: %1").arg(fileinfo_.fileName());
    QFile file(fileinfo_.filePath());
    if(!file.open(QIODevice::ReadOnly))
        return tr("failed to open %1: %2").arg(fileinfo_.fileName()).arg(file.errorString());
    const QByteArray contents(file.readAll());
    file.close();
    fileinfo_.refresh();
    if(contents.size() != fileinfo_.size())
        return tr("failed to read all of %1: %2").arg(fileinfo_.fileName()).arg(file.errorString());

    QString retv = readXML(contents);
    if(!retv.isEmpty())
        return retv;

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(contents.constData(), contents.size());
    sha1_ = hash.result();

    QSettings settings;
    settings.beginGroup( "plugins" );
    settings.beginGroup( fileInfo().completeBaseName() );
    enabled_ = settings.value("enabled").toBool();

    return QString();
}

void PluginInfo::setEnabled( bool b )
{
    QSettings settings;
    enabled_ = b;
    settings.beginGroup( "plugins" );
    settings.beginGroup( fileInfo().completeBaseName() );
    settings.setValue("enabled",enabled_);
}
