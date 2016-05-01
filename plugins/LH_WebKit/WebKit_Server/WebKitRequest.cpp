/**
  \file     WebKitRequest.cpp
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011, Johan Lindh

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

#include <QApplication>
#include <QPainter>
#include <QDebug>

#include "WebKitCommand.h"
#include "WebKitRequest.h"

WebKitRequest::WebKitRequest( QLocalSocket *sock, WebKitServerWindow *parent ) : QObject(parent), sock_(sock)
{
    page_ = new QWebEngineView(parent);
    // page_->setNetworkAccessManager( parent->manager() );
    // page_->mainFrame()->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
    // page_->mainFrame()->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );
    // page_->setViewportSize( QSize(0,0) );

    dirty_ = false;
    progress_ = 0;
    bytes_required_ = 0;

    connect( sock_, SIGNAL(readyRead()), this, SLOT(readyRead()) );
    connect( sock_, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(dying()) );
    connect( sock_, SIGNAL(disconnected()), this, SLOT(dying()) );

    // connect( page_, SIGNAL(repaintRequested(QRect)), this, SLOT(repaintRequested()) );
    connect( page_, SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)) );
    connect( page_, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)) );
}

WebKitRequest::~WebKitRequest()
{
    if( page_ )
    {
        delete page_;
        page_ = NULL;
    }
}

void WebKitRequest::checkSize()
{
    if( image_.size() != size() )
    {
        page_->resize(size());
        image_ = QImage( size(), QImage::Format_ARGB32_Premultiplied );
        image_.fill( qRgba(0,0,0,0) );
        dirty_ = true;
    }
}

template<typename Arg, typename R, typename C>
struct InvokeWrapper {
    R *receiver;
    void (C::*memberFun)(Arg);
    void operator()(Arg result) {
        (receiver->*memberFun)(result);
    }
};

template<typename Arg, typename R, typename C>
InvokeWrapper<Arg, R, C> invoke(R *receiver, void (C::*memberFun)(Arg))
{
    InvokeWrapper<Arg, R, C> wrapper = {receiver, memberFun};
    return wrapper;
}

void WebKitRequest::checkHtmlChange(const QString& s) {
  if (html() != s) {
    // page_->settings()->setObjectCacheCapacities(0,0,0);
    // page_->setHtml( html(), url() );
    html_ = s;
    url_ = page_->url();
    /*
    QString bgcolor = page_->findFirstElement("body").attribute("bgcolor","white");
    QPalette palette = page_->palette();
    palette.setBrush( QPalette::Base, QColor(bgcolor) );
    page_->setPalette( palette );
    page_->toHtml(invoke(this, &WebKitRequest::checkHtmlChange));
    */
  }
}


void WebKitRequest::setPage()
{
    checkSize();

    if( html().isEmpty() )
    {
        page_->load(url());
    }
    else
    {
        page_->setHtml(html());
    }

    page_->showMinimized();
}

void WebKitRequest::repaintRequested()
{
    dirty_ = true;
    return;
}

void WebKitRequest::repaint()
{
    QPainter painter;

    if( !page_ || server()->paused() ) return;

    if( page_->size() != image_.size() )
        page_->resize(image_.size());

    image_.fill( qRgba(0,0,0,0) );
    if( painter.begin(&image_) )
    {
        page_->render(&painter);
        painter.end();

        WebKitData data;
        data.bytecount = image_.byteCount();
        data.progress = progress_;
        data.w = image_.width();
        data.h = image_.height();

        if( sock_ && sock_->write( (const char*)(void*)&data, sizeof(data) ) == sizeof(data) )
        {
            if( sock_->write( (const char*)(void*)image_.bits(), data.bytecount ) == data.bytecount )
                sock_->flush();
        }

        dirty_ = false;
    }
}

void WebKitRequest::loadProgress(int p)
{
    progress_ = p;
    return;
}

void WebKitRequest::loadFinished(bool)
{
    progress_ = 100;
    dirty_ = true;
    return;
}

void WebKitRequest::readyRead()
{
    qint64 bytesread;

    if( server()->paused() ) return;

    while( sock_ && sock_->isReadable() )
    {
        if( bytes_required_ < 1 )
        {
            if( sock_->bytesAvailable() < (int) sizeof(qint32) ) return;
            bytesread = sock_->read( (char*)(void*)&bytes_required_, sizeof(bytes_required_) );
            Q_ASSERT( bytesread == sizeof(qint32) );
        }

        if( sock_->bytesAvailable() < bytes_required_ ) return;

        QByteArray array( bytes_required_, 0 );
        bytesread = sock_->read( array.data(), bytes_required_ );
        Q_ASSERT( bytesread ==  bytes_required_ );

        bytes_required_ = 0;
        WebKitCommand cmd( array );

        // if( cmd.command() != 'P' ) qDebug() << "COMMAND" << (char)(cmd.command()) << cmd.size() << cmd.url().toString() << cmd.html().size();

        switch( cmd.command() )
        {
        case 'F': // Form functionality
        case 'H': // static HTML
            size_ = cmd.size();
            url_ = cmd.url();
            html_ = cmd.html();
            setPage();
            break;
        case 'U': // URL
            size_ = cmd.size();
            url_ = cmd.url();
            html_.clear();
            setPage();
            break;
        case 'R': // Resize
            size_ = cmd.size();
            checkSize();
            break;
        case 'P': // Ping
            WebKitData data;
            memset( &data, 0, sizeof(data) );
            sock_->write( (const char*)(void*)&data, sizeof(data) );
            if( !sock_->waitForBytesWritten(2000) )
            {
                qWarning() << "WebKitServer: timed out writing P reply";
            }
            break;
        case 'S': // Show window
            server()->show();
            server()->raise();
            server()->activateWindow();
            break;
        case 's': // Hide window
            server()->hide();
            break;
        case 'Q': // Quit
            server()->close();
            break;
        default:
            qDebug() << "WebKitServer: bad command" << cmd.command();
            break;
        }
    }

    return;
}

void WebKitRequest::dying()
{
    sock_ = NULL;
    emit dead();
    disconnect(this);
}
