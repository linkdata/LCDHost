/**
  \file     WebKitRequest.h
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

#ifndef WEBKITREQUEST_H
#define WEBKITREQUEST_H

#include <QtWebEngine>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
# include <QtWebEngineWidgets/QtWebEngineWidgets>
#endif
#include <QtNetwork>

#include <QObject>
#include <QImage>
#include <QSharedMemory>

#include "WebKitServerWindow.h"
#include "WebKitCommand.h"

class WebKitRequest : public QObject
{
    Q_OBJECT

public:
    WebKitRequest( QLocalSocket *sock, WebKitServerWindow *parent );
    ~WebKitRequest();

    void checkSize();
    void setPage();

    QSize size() const { return size_; }
    QUrl url() const { return url_; }
    QString html() const { return html_; }
    QImage image() const { return image_; }
    bool isDirty() const { return dirty_; }
    void repaint();
    bool isDead() const { return sock_ == NULL; }

    WebKitServerWindow *server() const { return static_cast<WebKitServerWindow *>(parent()); }
    void checkHtmlChange(const QString& s);

public slots:
    void readyRead();
    void repaintRequested();
    void loadProgress(int);
    void loadFinished(bool);
    void dying();

signals:
    void dead();

private:
    qint32 bytes_required_;
    QLocalSocket *sock_;
    QSize size_;
    QUrl url_;
    QString html_;
    QImage image_;
    QWebEngineView *page_;
    QSharedMemory sm_;
    int progress_;
    bool dirty_;
};

#endif // WEBKITREQUEST_H
