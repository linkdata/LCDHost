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

#include "LH_WebKitURL.h"

LH_PLUGIN_CLASS(LH_WebKitURL)

lh_class *LH_WebKitURL::classInfo()
{
    static lh_class classinfo =
    {
        sizeof(lh_class),
        "Dynamic",
        "DynamicWebKitURL",
        "WebKit URL",
        96, 32,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classinfo;
}

#ifdef USE_NAM
LH_WebKitURL::LH_WebKitURL() : LH_WebKit(true)
#else
LH_WebKitURL::LH_WebKitURL() : LH_WebKit()
#endif
{
    setup_url_ = new LH_Qt_QString(this,"URL",QString(),LH_FLAG_FOCUS);
    connect( setup_url_, SIGNAL(changed()), this, SLOT(urlChanged()) );
    setup_url_sanitized_ = new LH_Qt_QString(this,"Sanitized URL",QString(),LH_FLAG_READONLY);

#ifdef USE_NAM
    nam_ = new QNetworkAccessManager(this);
    connect(nam_, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
#endif
}

void LH_WebKitURL::urlChanged()
{
    url_ = QUrl::fromUserInput( setup_url_->value() );
    if( url_.isValid() )
    {
        setup_url_sanitized_->setValue( url_.toString() );
#ifdef USE_NAM
        fetch();
#else
        sendRequest( url_ );
#endif
    }
    else setup_url_sanitized_->setValue( QString() );
    return;
}

#ifdef USE_NAM

void LH_WebKitURL::finished( QNetworkReply* reply )
{
    if( reply != NULL )
    {
        if( reply->error() == QNetworkReply::NoError )
        {
            if( reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 200 )
            {
                sendRequest( QUrl::fromLocalFile( QString::fromUtf8( state()->dir_layout ) + "/" ), reply->readAll() );
            }
            else if( reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 301 )
            {
                QString dest = reply->header(QNetworkRequest::LocationHeader).toString();
                qDebug() << "LH_WebKitURL: Changing URL, content permanently moved to" << dest;
                url_ = QUrl::fromUserInput( dest );
                setup_url_->setValue(dest);
                fetch();
            }
            else
            {
                qWarning() << "LH_WebKitURL: HTTP code" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << reply->errorString() << reply->url().toString();
            }
        }
        else
        {

        }
        reply->deleteLater();
    }
}

void LH_WebKitURL::fetch()
{
    //lastrefresh_ = QDateTime::currentDateTime();
    QNetworkProxyQuery npq(url_);
    QList<QNetworkProxy> listOfProxies = QNetworkProxyFactory::systemProxyForQuery(npq);
    if(listOfProxies.count()!=0)
        if(listOfProxies.at(0).type() != QNetworkProxy::NoProxy)
            nam_->setProxy(listOfProxies.at(0));

    nam_->get( QNetworkRequest(url_) );
}

#endif
