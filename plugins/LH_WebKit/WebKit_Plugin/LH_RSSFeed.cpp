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

#include "LH_RSSFeed.h"

#include <QDebug>
#include <QPainter>
#include <QDesktopServices>
#include <QNetworkProxyQuery>
#include <QNetworkReply>

LH_RSSFeed::LH_RSSFeed(QString feedUrl, int refreshTime, int delayTime) :
    nowshowing_(-1),
    switchwait_(0),
    nam_(0),
    // connectionId(0),
    refresh_(refreshTime),
    delay_(delayTime)
{
    nam_ = new QNetworkAccessManager(this);
    lastrefresh_ = QDateTime::currentDateTime();
    lastopen_.start();
    lastmove_.start();
    url_ = QUrl::fromUserInput(feedUrl);

    QObject::connect(nam_, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));

    if(feedUrl!="") fetch();

    notifyTimer_.start();
}

static RSSItem empty_rss_item = {"", "", "", "", "", {"",0,0}};

RSSItem LH_RSSFeed::item(int mod)
{
    if( nowshowing_ >= 0 && nowshowing_ < items_.size() )
    {
        int index = nowshowing_ + mod;
        while( index >= items_.size() ) index -= items_.size();
        while( index < 0 ) index += items_.size();

        return items_.at(index);
    }
    return empty_rss_item;
}

void LH_RSSFeed::openItem(int mod)
{
    if( lastopen_.elapsed() > 200 )
    {
        if( nowshowing_ >= 0 && nowshowing_ < items_.size() )
        {
            int index = nowshowing_ + mod;
            if( index >= items_.size() ) index = 0;
            if( index < 0 ) index = items_.size()-1;

            lastopen_.restart();
            QDesktopServices::openUrl( QUrl::fromUserInput(items_.at(index).link) );
        }
    }
}

void LH_RSSFeed::moveNext()
{
    if(lastmove_.elapsed()<=200) return;
    switchwait_ = 0;
    if( nowshowing_ >= 0 && nowshowing_ < items_.size() )
    {
        ++ nowshowing_;
        if( nowshowing_ >= items_.size() ) nowshowing_ = 0;
        lastmove_.restart();
        emit changed();
    }
}
void LH_RSSFeed::moveBack()
{
    if(lastmove_.elapsed()<=200) return;
    switchwait_ = 0;
    if( nowshowing_ >= 0 && nowshowing_ < items_.size() )
    {
        -- nowshowing_;
        if( nowshowing_ < 0 ) nowshowing_ = items_.size()-1;
        lastmove_.restart();
        emit changed();
    }
}

int LH_RSSFeed::refresh()
{
    return refresh_;
}
void LH_RSSFeed::setRefresh(int r)
{
    if(refresh_ != r)
    {
        refresh_ = r;
        //requestPolling();
        emit refreshChanged();
    }
}

int LH_RSSFeed::delay()
{
    return delay_;
}
void LH_RSSFeed::setDelay(int d)
{
    if(delay_ != d)
    {
        delay_ = d;
        //requestPolling();
        emit delayChanged();
    }
}

QString LH_RSSFeed::url()
{
    return url_.toString();
}
void LH_RSSFeed::setUrl(QString u)
{
    if(url_.toString() != u)
    {
        url_.setUrl( u );
        emit urlChanged();
        fetch();
    }
}


void LH_RSSFeed::connect(QObject* intrfce)
{
    if (!connections_.contains(intrfce))
        connections_.append(intrfce);
}

int LH_RSSFeed::connections()
{
    return connections_.count();
}

bool LH_RSSFeed::release(QObject* intrfce)
{
    if (connections_.contains(intrfce))
    {
        connections_.removeAt(connections_.indexOf(intrfce));
        return true;
    }
    else
        return false;
}

int LH_RSSFeed::notify(int code,void* param)
{
    Q_UNUSED(param);

    if( code & LH_NOTE_SECOND  && notifyTimer_.elapsed() >= 1000)
    {
        notifyTimer_.restart();
        if( lastrefresh_.addSecs(60*refresh_) < QDateTime::currentDateTime() )
        {
            lastrefresh_ = QDateTime::currentDateTime();
            fetch();
        }

        if( ++switchwait_ >= delay_ )
        {
            switchwait_ = 0;
            if( nowshowing_ >= 0 && nowshowing_ < items_.size() )
            {
                ++ nowshowing_;
                if( nowshowing_ >= items_.size() ) nowshowing_ = 0;
                emit changed();
            }
        }
    }
    return LH_NOTE_SECOND;
}

void LH_RSSFeed::fetch()
{
    emit begin();
    lastrefresh_ = QDateTime::currentDateTime();
    nowshowing_ = -1;
    switchwait_ = 0;
    items_.clear();
    xml_.clear();

    QNetworkProxyQuery npq(url_);
    QList<QNetworkProxy> listOfProxies = QNetworkProxyFactory::systemProxyForQuery(npq);
    if(listOfProxies.count()!=0)
        if(listOfProxies.at(0).type() != QNetworkProxy::NoProxy)
            nam_->setProxy(listOfProxies.at(0));

    nam_->get( QNetworkRequest(url_) );
}

void LH_RSSFeed::finished( QNetworkReply* reply )
{
    if( reply )
    {
        if( reply->error() == QNetworkReply::NoError )
        {
            if( reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 200 )
            {
                xml_.addData(reply->readAll());
                parseXml();
            }
            else if( reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 301 )
            {
                QString dest = reply->header(QNetworkRequest::LocationHeader).toString();
                qDebug() << "LH_RSSFeed: Changing URL, content permanently moved to" << dest;
                url_ = QUrl::fromUserInput( dest );
                emit urlChanged();
                fetch();
            }
            else
            {
                qWarning() << "LH_RSSFeed: HTTP code" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << reply->errorString() << reply->url().toString();
            }
        }
        else
        {

        }
        reply->deleteLater();
    }
    if( items_.size() > 0 )
    {
        nowshowing_ = 0;
        switchwait_ = 0;
        emit changed();
        emit finished();
    }
}

void LH_RSSFeed::parseXml()
{
    QString currentTag;
    RSSItem item;
    while (!xml_.atEnd())
    {
        xml_.readNext();
        if (xml_.isStartElement())
        {
            if( xml_.name() == "item" ) item.link = xml_.attributes().value("rss:about").toString();
            if( xml_.name() == "thumbnail" )
            {
                item.thumbnail.url = xml_.text().toString();
                if(xml_.attributes().hasAttribute("url"))
                    item.thumbnail.url = xml_.attributes().value("url").toString();
                if(xml_.attributes().hasAttribute("width"))
                    item.thumbnail.width = xml_.attributes().value("width").toString().toInt();
                if(xml_.attributes().hasAttribute("height"))
                    item.thumbnail.height = xml_.attributes().value("height").toString().toInt();
            }
            currentTag = xml_.name().toString();
        }
        else if (xml_.isEndElement())
        {
            if (xml_.name() == "item")
            {
                items_ << item;
                item = empty_rss_item;
            }
        }
        else if (xml_.isCharacters() && !xml_.isWhitespace())
        {
            if( currentTag == "title" ) item.title = xml_.text().toString();
            else if( currentTag == "link" ) item.link = xml_.text().toString();
            else if( currentTag == "description" ) item.description = xml_.text().toString();
            else if( currentTag == "author" ) item.author = xml_.text().toString();
            else if( currentTag == "pubDate" ) item.pubDate = xml_.text().toString();
        }
    }
    if (xml_.error() && xml_.error() != QXmlStreamReader::PrematureEndOfDocumentError)
    {
        qWarning() << "XML ERROR:" << xml_.lineNumber() << ": " << xml_.errorString();
    }
}



QHash<QString, LH_RSSFeed* >* rssFeeds = NULL;

