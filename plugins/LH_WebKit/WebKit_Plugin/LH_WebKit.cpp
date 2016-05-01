/**
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2010 Johan Lindh

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
#include <QtNetwork>
#include <QRegExp>

#include "LH_WebKit.h"

LH_WebKit::LH_WebKit(const bool enableParsing)
{
    parsingEnabled_ = enableParsing;
    int parseFlags = (!enableParsing? LH_FLAG_HIDDEN | LH_FLAG_READONLY | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK : 0);

    zoom_ = new LH_Qt_QSlider(this,"Zoom",10,1,20,LH_FLAG_FOCUS);
    zoom_->setOrder(1);
    connect( zoom_, SIGNAL(change(int)), this, SLOT(zoomChanged(int)) );

    progress_ = new LH_Qt_QProgressBar(this,"~WebKitProgress",0,0,100,LH_FLAG_READONLY|LH_FLAG_HIDDEN);
    progress_->setOrder(2);

    sock_ = NULL;
    lastpong_ = QTime::currentTime();
    sent_html_ = false;
    memset( &kitdata_, 0, sizeof(kitdata_) );

    setup_parse_ = new LH_Qt_bool(this, "Enable Parsing", false, parseFlags | LH_FLAG_LAST);
    setup_parse_->setHelp("Enabling this allows you to extract or reformat the html data.<br><br>N.B.: This requires an understanding of regular expressions and HTML.");
    connect( setup_parse_, SIGNAL(changed()), this, SLOT(reparse()) );

    setup_regexp_lazy_ = new LH_Qt_bool(this, "Lazy Regular Expressions", false, parseFlags | LH_FLAG_HIDDEN | LH_FLAG_LAST);
    setup_regexp_lazy_->setHelp("QT's regex engine does not support the \"?\" OR \"lazy\" modifier. Instead you can only set ALL wildcards as being either greedy or lazy.");
    connect( setup_regexp_lazy_, SIGNAL(changed()), this, SLOT(reparse()) );

    setup_regexp_ = new LH_Qt_QTextEdit(this, "Parsing Expression", "(.*)", parseFlags | LH_FLAG_HIDDEN | LH_FLAG_LAST);
    setup_regexp_->setHelp("This is a \"Regular Expression\" and parses the entire web page. Use capture groups to extract data. Note that the expression is only applied once; also as this is a regualar expression, be careful to ensure there are no undesired trailing spaces or carridge returns etc as they will affect the matching.");
    connect( setup_regexp_, SIGNAL(changed()), this, SLOT(reparse()) );

    setup_template_ = new LH_Qt_QTextEdit(this, "Parsing Template", "\\1", parseFlags | LH_FLAG_HIDDEN | LH_FLAG_LAST);
    setup_template_->setHelp("This contains the html that will be displayed with any tokens from the regular expression.<br>"
                             "<br>Tokens:"
                             "<br><b>\\n</b> : To place the first capture group in the template, use the token \\1, the second one \\2, the third is \\3 etc."
                             "<br><b>\\layout_path</b> : This token represents the path of the currently loaded layout; "
                             "<br>e.g. &lt;img src=\"file:///\\layout_path/Wallpaper.png\"&gt;"
                             );
    connect( setup_template_, SIGNAL(changed()), this, SLOT(reparse()) );
    parseThread = new LH_ParseThread(this);
    connect( parseThread, SIGNAL(finished()), this, SLOT(doneParsing()));
    return;
}

LH_WebKit::~LH_WebKit()
{
    if( sock_ )
    {
        if( sock_->state() == QLocalSocket::ConnectedState ) sock_->abort();
        delete sock_;
        sock_ = NULL;
    }
}

const char *LH_WebKit::userInit()
{
    return 0;
}

void LH_WebKit::userTerm()
{
}


void LH_WebKit::zoomChanged(int)
{
    if( verifySocket() ) sendData(true);
    requestRender();
}

int LH_WebKit::notify(int code, void *param)
{
    Q_UNUSED(param);

    if( code & LH_NOTE_SECOND )
    {
        if( verifySocket() ) WebKitCommand( 'P', QSize(0,0) ).write(sock_);
    }

    return LH_NOTE_SECOND;
}

void LH_WebKit::socketTimeout()
{
    lastpong_ = QTime::currentTime();
    sent_html_ = false;
    sock_->abort();
}

bool LH_WebKit::verifySocket()
{
    if( sock_ == NULL )
    {
        sock_ = new QLocalSocket();
        connect( sock_, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(error(QLocalSocket::LocalSocketError)) );
        connect( sock_, SIGNAL(connected()), this, SLOT(connected()) );
        connect( sock_, SIGNAL(disconnected()), this, SLOT(disconnected()) );
        connect( sock_, SIGNAL(readyRead()), this, SLOT(readyRead()) );
    }

    if( sock_ )
    {
        switch( sock_->state() )
        {
        case QLocalSocket::UnconnectedState:
            lastpong_ = QTime::currentTime();
            sent_html_ = false;
            sock_->connectToServer("LCDHost_WebKitServer");
            break;
        case QLocalSocket::ConnectingState:
            if( lastpong_.elapsed() < 2000 ) return false;
            qDebug() << "LH_WebKit: can't connect to WebKitServer, retrying";
            socketTimeout();
            break;
        case QLocalSocket::ConnectedState:
            if( lastpong_.elapsed() < 10000 ) return true;
            qDebug() << "LH_WebKit: WebKitServer unresponsive, reconnecting";
            socketTimeout();
            break;
        case QLocalSocket::ClosingState:
            if( lastpong_.elapsed() < 1000 ) return false;
            // this'll leak, but deleting it will cause heap corruption
            qWarning() << "LH_WebKit: connection hung, abandoning it";
            disconnect( sock_, 0, 0, 0 );
            socketTimeout();
            sock_ = NULL;
            break;
        }
    }

    return false;
}

void LH_WebKit::connected()
{
    lastpong_ = QTime::currentTime();
    sendData(false);
    return;
}

void LH_WebKit::disconnected()
{
    lastpong_ = QTime::currentTime();
    sent_html_ = false;
    memset( &kitdata_, 0, sizeof(kitdata_) );
    return;
}

void LH_WebKit::error(QLocalSocket::LocalSocketError err)
{
    if(err == QLocalSocket::ServerNotFoundError || err == QLocalSocket::ConnectionRefusedError)
    {
        static_cast<LH_QtPlugin_WebKit*>(lh_plugin())->startServer();
        return;
    }

    if( sock_ )
    {
        qWarning() << "LH_WebKit socket error" << (int)err << sock_->errorString();
        lastpong_ = QTime::currentTime();
        sent_html_ = false;
        memset( &kitdata_, 0, sizeof(kitdata_) );
        sock_->abort();
    }

    return;
}

QSize LH_WebKit::scaled_size()
{
    QSize _scaled_size;
    _scaled_size.setWidth( size_.width() * 10 / zoom_->value() );
    _scaled_size.setHeight( size_.height() * 10 / zoom_->value() );
    return _scaled_size;
}

void LH_WebKit::sendData( bool resize )
{
    if( sock_ && sock_->state() == QLocalSocket::ConnectedState )
    {
        if( size_.isValid() )
        {
            if( resize && sent_html_ )
            {
                // qDebug() << "LH_WebKit: sending resize" << scaled_size;
                WebKitCommand('R', scaled_size()).write(sock_);
            }
            else
                if(parsingEnabled_)
                {
                    if(parseThread->isRunning())
                    {
                        WebKitCommand(0, scaled_size(),url_,"Aborting...").write(sock_);
                        parseThread->terminate();
                    }
                    WebKitCommand(0, scaled_size(),url_,"Parsing...").write(sock_);
                    parseThread->parsedHtml = setup_template_->value();
                    parseThread->sourceHtml = html_;
                    parseThread->regex = setup_regexp_->value();
                    parseThread->isLazy = setup_regexp_lazy_->value();
                    parseThread->doParse = setup_parse_->value();
                    parseThread->tokensList = getTokens();

                    parseThread->start();
                }
                else
                {
                    WebKitCommand(0, scaled_size(),url_,html_).write(sock_);
                    sent_html_ = true;
                }
        }
    }
}

QHash<QString, QString> LH_WebKit::getTokens()
{
    QHash<QString, QString> tokens;

    QString layoutPath = state()->dir_layout;
    if (layoutPath.endsWith('\\') || layoutPath.endsWith('/'))
        layoutPath = layoutPath.left(layoutPath.length()-1);
    tokens.insert( "layout_path", layoutPath );

    return tokens;
}

void LH_WebKit::sendRequest( QUrl url, QString html )
{
    url_ = url;
    html_ = html;
    if( verifySocket() ) sendData(false);
    return;
}

QImage *LH_WebKit::render_qimage(int w, int h)
{
    if(QImage *img = initImage(w, h))
    {
        if(size_ != img->size())
        {
            size_ = img->size();
            img->fill(qRgba(0, 0, 0, 0));
            if(verifySocket()) sendData(true);
        }
        return img;
    }
    return 0;
}

void LH_WebKit::readyRead()
{
    if( sock_ == NULL || sock_->state() != QLocalSocket::ConnectedState ) return;

    if( kitdata_.bytecount == 0 )
    {
        if( sock_->bytesAvailable() < (int) sizeof(WebKitData) ) return;
        if( sock_->read( (char*)(void*)&kitdata_, sizeof(kitdata_) ) != sizeof(WebKitData) )
        {
            sock_->abort();
            return;
        }
        if( !kitdata_.bytecount && !kitdata_.w && !kitdata_.h )
        {
            // PONG
            lastpong_ = QTime::currentTime();
            return;
        }
    }

    if( sock_->bytesAvailable() < kitdata_.bytecount ) return;
    QByteArray bits = sock_->read(kitdata_.bytecount);
    Q_ASSERT( bits.size() == kitdata_.bytecount );

    if( progress_->value() != kitdata_.progress )
    {
        progress_->setFlag( LH_FLAG_HIDDEN, (kitdata_.progress == 0 || kitdata_.progress == 100) );
        progress_->setValue( kitdata_.progress );
    }
    if(hasImage())
    {
        QImage tmp( (uchar*)bits.data(), kitdata_.w, kitdata_.h, QImage::Format_ARGB32_Premultiplied );
        if( size() == tmp.size() ) *image() = tmp.copy();
        else *image() = tmp.scaled( size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    }

    lastpong_ = QTime::currentTime();
    memset( &kitdata_, 0, sizeof(kitdata_) );
    callback( lh_cb_render, 0 );
}

void LH_WebKit::reparse()
{
    setup_regexp_->setFlag(LH_FLAG_HIDDEN, !setup_parse_->value());
    setup_regexp_lazy_->setFlag(LH_FLAG_HIDDEN, !setup_parse_->value());
    setup_template_->setFlag(LH_FLAG_HIDDEN, !setup_parse_->value());
    sent_html_ = false;
    sendData(false);
    requestRender();
}

void LH_WebKit::doneParsing()
{
    // qDebug() << "LH_WebKit: sending data" << scaled_size << url_ << html_.size();
    WebKitCommand(0, scaled_size(),url_,parseThread->parsedHtml).write(sock_);
    sent_html_ = true;
}

void LH_ParseThread::run()
{
    if (doParse)
    {
        QRegExp rx(regex, Qt::CaseInsensitive, QRegExp::RegExp2 );
        rx.setMinimal(isLazy);
        if (rx.indexIn(sourceHtml)!=-1)
            for(int i=1; i <= rx.captureCount(); i++)
                parsedHtml = parseToken(parsedHtml, QString::number(i), rx.cap(i), "0-9" );
        for(int i=0; i<tokensList.count(); i++)
        {
            QString key = tokensList.keys().at(i);
            parsedHtml = parseToken(parsedHtml, key, tokensList.value(key) );
        }
    }
    else
        parsedHtml = sourceHtml;
}

QString LH_ParseThread::parseToken(QString beforeParsing, QString token, QString value, QString lookAheadChars)
{
    QString regExp = QString("\\\\%1(?=[^%2]|$)").arg(token).arg(lookAheadChars);
    return beforeParsing.replace(QRegExp(regExp, Qt::CaseInsensitive, QRegExp::RegExp2), value  );
}
