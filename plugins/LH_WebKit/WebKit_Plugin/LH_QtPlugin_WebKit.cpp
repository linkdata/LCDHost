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

#include <QProcess>
#include <QtNetwork>
#include <QFileInfo>

#include "LH_RSSInterface.h"
#include "LH_QtPlugin_WebKit.h"
#include "WebKitCommand.h"

LH_PLUGIN(LH_QtPlugin_WebKit)

char __lcdhostplugin_xml[] =
"<?xml version=\"1.0\"?>"
"<lcdhostplugin>"
  "<id>WebKit</id>"
  "<rev>" STRINGIZE(REVISION) "</rev>"
  "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
  "<ver>" "r" STRINGIZE(REVISION) "</ver>"
  "<versionurl>http://www.linkdata.se/lcdhost/version.php?arch=$ARCH</versionurl>"
  "<author>Johan \"SirReal\" Lindh</author>"
  "<homepageurl><a href=\"http://www.linkdata.se/software/lcdhost\">Link Data Stockholm</a></homepageurl>"
  "<logourl></logourl>"
  "<shortdesc>"
  "Allows HTML and JavaScript."
  "</shortdesc>"
  "<longdesc>"
  "<a href=\"http://webkit.org\">WebKit</a> integration. Add URL's or HTML instances, complete with JavaScript."
  "</longdesc>"
"</lcdhostplugin>";

bool LH_QtPlugin_WebKit::startServer()
{
    if( last_start_.isNull() || last_start_.elapsed() > 10000 )
    {
        last_start_ = QTime::currentTime();
        QString wksname = QFileInfo(QCoreApplication::applicationFilePath()).canonicalPath() + "/WebKitServer";
        QStringList wksargs;
        wksargs << "--hidden"
               << "--datadir"
               << QString::fromUtf8(state()->dir_data)
               << "--plugindir"
               << QString::fromUtf8(state()->dir_plugins)
               << "--verbose";
        qDebug() << "LH_WebKit: attempting to start" << wksname << wksargs;
        QProcess::startDetached( wksname, wksargs );
    }
    return true;
}

bool LH_QtPlugin_WebKit::sendQuit()
{
    QLocalSocket sock;
    sock.connectToServer("LCDHost_WebKitServer");
    if( !sock.waitForConnected(100) ) return false;
    WebKitCommand('Q').write(&sock);
    sock.waitForBytesWritten(100);
    sock.disconnectFromServer();
    return true;
}

void LH_QtPlugin_WebKit::term()
{
    sendQuit();

    if(rssFeeds != NULL)
    {
        while( !rssFeeds->isEmpty() )
        {
            QString url = rssFeeds->keys().at(0);
            LH_RSSFeed* feed = rssFeeds->take(url);
            if( feed ) delete feed;
        }
        delete rssFeeds;
        rssFeeds = NULL;
    }

    LH_QtPlugin::term();
}
