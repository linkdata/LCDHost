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

#ifndef LH_WEBKIT_H
#define LH_WEBKIT_H

#include <QProcess>
#include <QtNetwork>
#include <QThread>

#include "LH_QtPlugin_WebKit.h"

#include "LH_QtPlugin.h"
#include "LH_QtInstance.h"
#include "LH_Qt_QSlider.h"
#include "LH_Qt_QProgressBar.h"
#include "LH_Qt_bool.h"
#include "LH_Qt_QTextEdit.h"
#include "WebKitCommand.h"

class LH_ParseThread : public QThread {
    Q_OBJECT
protected:
    QString parseToken(QString beforeParsing, QString token, QString value, QString lookAheadChars = "0-9a-zA-Z_" );
public:
    LH_ParseThread( LH_QtObject *parent ) : QThread( parent ) { return; }
    QString parsedHtml;
    QString sourceHtml;
    QString regex;
    QHash<QString,QString> tokensList;
    bool isLazy;
    bool doParse;

    virtual void run();
};

class LH_WebKit : public LH_QtInstance
{
    Q_OBJECT

    QSize size_;
    QUrl url_;
    QLocalSocket *sock_;
    QTime lastpong_;
    bool sent_html_;
    bool parsingEnabled_;
    WebKitData kitdata_;

    LH_ParseThread *parseThread;

    void sendData(bool resize = false );

    QSize scaled_size();

protected:
    QString html_;

    LH_Qt_bool *setup_parse_;
    LH_Qt_QTextEdit *setup_regexp_;
    LH_Qt_bool *setup_regexp_lazy_;
    LH_Qt_QTextEdit *setup_template_;

    LH_Qt_QSlider *zoom_;
    LH_Qt_QProgressBar *progress_;

    QHash<QString, QString> virtual getTokens();
public:
    LH_WebKit( const bool enableParsing = false);
    ~LH_WebKit();

    const char *userInit();
    void userTerm();

    int notify(int, void *);
    QImage *render_qimage(int w, int h);

    QSize size() const { return size_; }
    bool verifySocket();
    void sendRequest( QUrl url, QString html = QString() );

    void socketTimeout();

public slots:
    void zoomChanged(int);
    void readyRead();
    void error(QLocalSocket::LocalSocketError);
    void connected();
    void disconnected();
    void reparse();
    void doneParsing();
};

#endif // LH_WEBKIT_H
