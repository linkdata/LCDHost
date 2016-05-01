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
#include <QCoreApplication>
#include <QDebug>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkProxyFactory>
#include <QProgressDialog>
#include <QCryptographicHash>
#include <QTime>

#include "AppSendVersionInfo.h"

// Send version info to http://www.linkdata.se/lcdhost/version.php
QString AppSendVersionInfo( QNetworkAccessManager& nam,
                        QString user,
                        QString pass,
                        QString id,
                        QString arch,
                        int r,
                        int amaj,
                        int amin,
                        QString url,
                        QString sha1
                        )
{
    QNetworkReply *reply;
    QByteArray response;
    QProgressDialog database_update_dlg( QObject::tr("Updating database"),QObject::tr("Cancel"),0,0);
    QCryptographicHash hash(QCryptographicHash::Md5);
    QString pw_md5;
    QByteArray saltary;
    QString salt;
    QString saltedpw;

    qsrand( QTime().msecsTo(QTime::currentTime()) );
    for( int i=0; i<16; i++) saltary.append( (uchar)(qrand()) );
    salt = saltary.toHex();
    hash.reset();
    hash.addData( pass.toLatin1() );
    pw_md5 = hash.result().toHex();
    hash.reset();
    hash.addData( salt.toLatin1() );
    hash.addData( pw_md5.toLatin1() );
    saltedpw = hash.result().toHex();

    QUrl query_url(
                QUrl::fromUserInput(
                    QString("http://www.linkdata.se/lcdhost/version.php?user=%1&salt=%2&pass=%3&id=%4&arch=%5&r=%6&api=%7.%8&url=%9&sha1=%10")
                    .arg(user).arg(salt).arg(saltedpw)
                    .arg(id).arg(arch).arg(r).arg(amaj).arg(amin)
                    .arg(url).arg(sha1)
                    )
                );

    QObject::connect(&nam, SIGNAL(finished(QNetworkReply*)), &database_update_dlg, SLOT(accept()));
    reply = nam.get( QNetworkRequest(query_url) );

    if( reply )
    {
        database_update_dlg.setMinimumDuration(1000);
        while( reply->isRunning() && !database_update_dlg.wasCanceled() )
            qApp->processEvents();
        response = reply->readAll();
    }

    if( response.size() != 2 || memcmp( "OK", response.constData(), 2 ) )
    {
        qCritical() << "Failed to update version database:";
        qCritical() << query_url.toString();
        return QString::fromLatin1(response);
    }

    return QString();
}

QString AppRevisionToVersion( int revision )
{
    int a, b, c;
    a = b = c = 0;
    while( revision >= 10000 ) { ++a; revision -= 10000; }
    while( revision >= 100 ) { ++b; revision -= 100; }
    c = revision;
    return QString("%1.%2.%3").arg(a).arg(b).arg(c);
}

int AppVersionToRevision( QString v )
{
    int revision = 0;
    v.replace('.','-');
    QStringList sl = v.split('-');
    foreach( QString s, sl )
    {
        bool ok = false;
        int r = s.toInt( &ok );
        if( ok )
        {
            Q_ASSERT( r < 100 );
            revision *= 100;
            revision += r;
        }
    }
    return revision;
}
