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

#include <QtGlobal>
#include <QDebug>
#include <QApplication>
#include <QDir>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include "AppState.h"

static void copydir( QXmlStreamWriter& xml, QDir fromdir, QString topath )
{
    QString rmdir_this;
    if( !topath.endsWith('/') ) topath.append('/');
    QDir datadir(AppState::instance()->dir_data());
    QDir todir(topath);

    if( !todir.exists() )
    {
        todir.mkdir(topath);
        rmdir_this = topath;
    }

    foreach( QFileInfo fi, fromdir.entryInfoList(QDir::Dirs|QDir::Files|QDir::Hidden|QDir::NoDotAndDotDot) )
    {
        if( fi.isDir() )
        {
            copydir( xml, QDir( fi.filePath() ), topath + fi.fileName() );
        }
        else if( fi.isFile() )
        {
            QFileInfo dfi( todir, fi.fileName() );
            if( QFile::copy( fi.filePath(), dfi.filePath() ) )
            {
                xml.writeEmptyElement("rm");
                xml.writeAttribute("name", datadir.relativeFilePath(dfi.filePath()) );
            }
            else
            {
                qDebug() << "AppOSXInstall failed to copy" << fi.filePath() << "to" << dfi.filePath();
            }
        }
        else
        {
            qDebug() << "AppOSXInstall don't know what to do with" << fi.filePath();
        }
    }

    if( !rmdir_this.isEmpty() )
    {
        xml.writeEmptyElement("rmdir");
        xml.writeAttribute("name", datadir.relativeFilePath(rmdir_this) );
    }
}

void AppOSXInstall()
{
    QDir datadir(AppState::instance()->dir_data());
    QDir skeldir( AppState::instance()->dir_binaries()+"../Skeleton");
    QFile manifest(AppState::instance()->dir_data()+".manifest.xml");

    if( !skeldir.exists() )
    {
        qDebug() << "AppOSXInstall can't find ../Skeleton";
        return;
    }

    if( manifest.open(QIODevice::ReadOnly) )
    {
        QXmlStreamReader xmlin(&manifest);
        while( !xmlin.atEnd() )
        {
            xmlin.readNext();
            if( xmlin.hasError() ) break;
            if( xmlin.isStartElement() )
            {
                if( xmlin.name() == "lcdhost" )
                {
                    if( xmlin.attributes().value("changeset").toString() == CHANGESET )
                        return;
                }
                if( xmlin.name() == "rm" )
                {
                    QFileInfo fi(datadir,xmlin.attributes().value("name").toString());
                    if( !QFile::remove( fi.canonicalFilePath() ) )
                        qDebug() << "AppOSXInstall failed to remove old file"<<fi.canonicalFilePath();
                }
                if( xmlin.name() == "rmdir" )
                {
                    QDir dir(datadir);
                    QString name(xmlin.attributes().value("name").toString());
                    if( !dir.rmdir(name) )
                        qDebug() << "AppOSXInstall failed to remove old directory"<<name;
                }
            }
        }
        if( xmlin.hasError() )
            qWarning() << "AppOSXInstall found an error in .manifest.xml:" << xmlin.errorString();
        xmlin.clear();
        manifest.close();
    }

    if( manifest.open(QIODevice::WriteOnly|QIODevice::Truncate) )
    {
        QXmlStreamWriter xmlout(&manifest);
        xmlout.setAutoFormatting(true);
        xmlout.writeStartDocument();
        xmlout.writeStartElement("lcdhost");
        xmlout.writeAttribute("version",VERSION);
        xmlout.writeAttribute("changeset",CHANGESET);
        xmlout.writeAttribute("installed",QDateTime::currentDateTime().toString(Qt::ISODate));
        copydir( xmlout, skeldir, AppState::instance()->dir_data() );
        xmlout.writeEndElement();
        xmlout.writeEndDocument();
        manifest.close();
    }
    else
        qWarning() << "AppOSXInstall can't create manifest" << manifest.fileName();
}
