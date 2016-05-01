/**
  \file     LH_TextRSS.cpp
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009 Johan Lindh

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Link Data Stockholm nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.


  */

#include <QDebug>
#include <QPainter>
#include <QDesktopServices>
#include <QNetworkProxyQuery>
#include <QNetworkReply>
#include "LH_TextRSS.h"

LH_PLUGIN_CLASS(LH_TextRSS)

lh_class *LH_TextRSS::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "Dynamic",
        "DynamicRSSFeedText",
        "RSS feed (Text)",
        -1, -1,
        lh_instance_calltable_NULL
    };

    if( classInfo.width == -1 )
    {
        QFont arial10("Arial",10);
        QFontMetrics fm( arial10 );
        classInfo.height = fm.height();
        classInfo.width = classInfo.height * 4;
    }

    return &classInfo;
}

LH_TextRSS::LH_TextRSS(const char *name) : LH_Text(name)
{
    lastrefresh_ = QDateTime::currentDateTime();
    lastopen_.start();

    nam_ = new QNetworkAccessManager(this);

    // Hide inherited attributes we don't use
    //setup_text_->setFlag( LH_FLAG_HIDDEN, true );
    setup_horizontal_->setFlag( LH_FLAG_HIDDEN, true );
    setup_vertical_->setFlag( LH_FLAG_HIDDEN, true );
    setup_scrollrate_->setFlag( LH_FLAG_HIDDEN, true );
    setup_scrollstep_->setFlag( LH_FLAG_HIDDEN, true );

    setup_url_ = new LH_Qt_QString(this,"URL",QString());
    setup_url_->setOrder(-1);
    connect( setup_url_, SIGNAL(changed()), this, SLOT(fetch()));

    setup_method_ = NULL;

    setup_delay_ = new LH_Qt_QSlider(this,tr("Switch delay"),5,1,10);

    setup_browser_ = new LH_Qt_InputState(this,tr("Open in browser"),QString(),LH_FLAG_AUTORENDER);
    connect( setup_browser_, SIGNAL(input(QString,int,int)), this, SLOT(openBrowser(QString,int,int)) );

    setup_prev_ = new LH_Qt_InputState(this,tr("Previous Headline"),QString(),LH_FLAG_AUTORENDER);
    connect( setup_prev_, SIGNAL(input(QString,int,int)), this, SLOT(prevHeadline(QString,int,int)) );

    setup_next_ = new LH_Qt_InputState(this,tr("Next Headline"),QString(),LH_FLAG_AUTORENDER);
    connect( setup_next_, SIGNAL(input(QString,int,int)), this, SLOT(nextHeadline(QString,int,int)) );

    setup_refresh_ = new LH_Qt_int(this,tr("Refresh (minutes)"),5);
    connect( setup_refresh_, SIGNAL(changed()), this, SLOT(requestPolling()) );

    connect(nam_, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
}

void LH_TextRSS::openBrowser(QString key,int flags,int value)
{
    Q_UNUSED(key);
    Q_UNUSED(flags);
    Q_UNUSED(value);
    if( lastopen_.elapsed() > 1000 )
    {
        if( nowshowing_ >= 0 && nowshowing_ < links_.size() )
        {
            lastopen_.restart();
            QDesktopServices::openUrl( QUrl::fromUserInput(links_.at(nowshowing_)) );
        }
    }
}

void LH_TextRSS::prevHeadline(QString key,int flags,int value)
{
    Q_UNUSED(key);
    Q_UNUSED(flags);
    Q_UNUSED(value);
    switchwait_ = 0;
    if( nowshowing_ >= 0 && nowshowing_ < titles_.size() )
    {
        -- nowshowing_;
        if( nowshowing_ < 0 ) nowshowing_ = titles_.size()-1;
        setRssItem();
    }
}

void LH_TextRSS::nextHeadline(QString key,int flags,int value)
{
    Q_UNUSED(key);
    Q_UNUSED(flags);
    Q_UNUSED(value);
    switchwait_ = 0;
    if( nowshowing_ >= 0 && nowshowing_ < titles_.size() )
    {
        ++ nowshowing_;
        if( nowshowing_ >= titles_.size() ) nowshowing_ = 0;
        setRssItem();
    }
}

void LH_TextRSS::fetch()
{
    lastrefresh_ = QDateTime::currentDateTime();
    setText("Loading RSS feed");
    nowshowing_ = -1;
    switchwait_ = 0;
    titles_.clear();
    links_.clear();
    xml_.clear();
    QUrl url = QUrl::fromUserInput(setup_url_->value());

    QNetworkProxyQuery npq(url);
    QList<QNetworkProxy> listOfProxies = QNetworkProxyFactory::systemProxyForQuery(npq);
    if(listOfProxies.count()!=0)
        if(listOfProxies.at(0).type() != QNetworkProxy::NoProxy)
            nam_->setProxy(listOfProxies.at(0));

    nam_->get( QNetworkRequest(url) );
}

void LH_TextRSS::finished( QNetworkReply* reply )
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
                qDebug() << "LH_TextRSS: Changing URL, content permanently moved to" << dest;
                setup_url_->setValue( dest );
                fetch();
            }
            else
            {
                qWarning() << "LH_TextRSS: HTTP code" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << reply->errorString() << reply->url().toString();
            }
        }
        else
        {

        }
        reply->deleteLater();
    }
    if( titles_.size() > 0 )
    {
        nowshowing_ = 0;
        switchwait_ = 0;
        setRssItem();
    }
}

int LH_TextRSS::notify(int code,void* param)
{
    if( code & LH_NOTE_SECOND )
    {
        if( lastrefresh_.addSecs(60*setup_refresh_->value()) < QDateTime::currentDateTime() )
        {
            lastrefresh_ = QDateTime::currentDateTime();
            fetch();
        }

        if( ++switchwait_ >= setup_delay_->value() )
        {
            switchwait_ = 0;
            if( nowshowing_ >= 0 && nowshowing_ < titles_.size() )
            {
                ++ nowshowing_;
                if( nowshowing_ >= titles_.size() ) nowshowing_ = 0;
                setRssItem();
            }
        }
    }
    return LH_NOTE_SECOND | LH_Text::notify(code,param);
}

void LH_TextRSS::setRssItem()
{
    if( nowshowing_ >= 0 && nowshowing_ < titles_.size() )
    {
        if( setText(titles_.at(nowshowing_)) ) requestRender();
    }
}

void LH_TextRSS::parseXml()
{
    while (!xml_.atEnd())
    {
        xml_.readNext();
        if (xml_.isStartElement())
        {
            if( xml_.name() == "item" ) linkString = xml_.attributes().value("rss:about").toString();
            currentTag = xml_.name().toString();
        }
        else if (xml_.isEndElement())
        {
            if (xml_.name() == "item")
            {
                titles_ << titleString;
                links_ << linkString;
                titleString.clear();
                linkString.clear();
            }
        }
        else if (xml_.isCharacters() && !xml_.isWhitespace())
        {
            if( currentTag == "title" ) titleString = xml_.text().toString();
            else if( currentTag == "link" ) linkString = xml_.text().toString();
        }
    }
    if (xml_.error() && xml_.error() != QXmlStreamReader::PrematureEndOfDocumentError)
    {
        qWarning() << "XML ERROR:" << xml_.lineNumber() << ": " << xml_.errorString();
    }
}

QImage *LH_TextRSS::render_qimage( int w, int h )
{
    if( !prepareForRender(w,h) ) return NULL;

    QPainter painter;
    if( painter.begin(image_) )
    {
        QRectF target;

        target.setSize( textimage().size() );
        target.moveLeft( image_->width()/2 - textimage().width()/2 );
        target.moveTop( image_->height()/2 - textimage().height()/2 );

        if( textimage().width() > image_->width() )
            target.moveLeft( 0 );

        painter.drawImage( target, textimage(), textimage().rect() );
        painter.end();
    }

    return image_;
}
