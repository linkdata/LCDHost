/**
  \file     LH_QtPlugin_TS3.cpp
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  \legalese
    This module is based on original work by Johan Lindh.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

  */

#include "LH_QtPlugin_TS3.h"

LH_PLUGIN(LH_QtPlugin_TS3)

char __lcdhostplugin_xml[] =
"<?xml version=\"1.0\"?>"
"<lcdhostplugin>"
  "<id>TS3</id>"
  "<rev>" STRINGIZE(REVISION) "</rev>"
  "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
  "<ver>" STRINGIZE(VERSION) "\nr" STRINGIZE(REVISION) "</ver>"
  "<versionurl>http://www.linkdata.se/lcdhost/version.php?arch=$ARCH</versionurl>"
  "<author>Andy \"Triscopic\" Bridges</author>"
  "<homepageurl><a href=\"http://www.codeleap.co.uk\">CodeLeap</a></homepageurl>"
  "<logourl></logourl>"
  "<shortdesc>"
    "Enables interaction with TeamSpeak 3."
  "</shortdesc>"
  "<longdesc>"
    "<p>This plugin allows LCDHost to receive information from a running copy of TeamSpeak 3, such as the identity of people currently speaking.</p>"
    "<p>It can also display your current microphone / speak mute status, the name of the channel you are connected to and more.</p>"
  "</longdesc>"
"</lcdhostplugin>";

const char *LH_QtPlugin_TS3::userInit()
{
    if( const char *err = LH_QtPlugin::userInit() ) return err;

    server_action_ = sa_disconnected;
    tryConnectTimer_.start();

    socket_ = new QTcpSocket(this);
    myclid_ = -1;

    setup_connection_details_ = new LH_Qt_QString(this, "^ConnectionDetails", QString(), LH_FLAG_NOSINK | LH_FLAG_NOSOURCE, lh_type_string_html );
    setup_talking_details_ = new LH_Qt_QString(this, "~hr1", QString(), LH_FLAG_NOSINK | LH_FLAG_NOSOURCE, lh_type_string_html );
    new LH_Qt_QString( this, "~hr2", "<hr/>", LH_FLAG_NOSINK | LH_FLAG_NOSOURCE , lh_type_string_html);

#ifdef TS3_USER_DEFINED_UID
    setup_nickname_expression_ = new LH_Qt_QString(this, "Nickname Expression", "", LH_FLAG_NOSINK | LH_FLAG_NOSOURCE);
    //setup_nickname_expression_->setTitle("Nickname:");
    setup_nickname_expression_->setHelp("Entering your nickname will enable the plugin to acquire additional information about your status.<br/><br/>Note that this field is actually a Regular Expression, so you can have it match multiple possible names. The first match it finds will be the one it uses.");
    connect(setup_nickname_expression_, SIGNAL(changed()), this, SLOT(updateMyDetails()));
#endif

    setup_user_detail_ = new LH_Qt_QString(this, "~hr3", "", LH_FLAG_NOSINK | LH_FLAG_NOSOURCE , lh_type_string_html);

    setup_nickname_ = new LH_Qt_QString(this, "Nickname", "", LH_FLAG_HIDDEN | LH_FLAG_READONLY | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSINK);
    setup_nickname_->setPublishPath("/3rdParty/TeamSpeak 3/Nickname");

    setup_talking_ = new LH_Qt_QString(this, "Speaking", "", LH_FLAG_HIDDEN | LH_FLAG_READONLY | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSINK);
    setup_talking_->setPublishPath("/3rdParty/TeamSpeak 3/Speaking");

    setup_talking_me_ = new LH_Qt_bool(this, "Self Speaking", false, LH_FLAG_HIDDEN | LH_FLAG_READONLY | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSINK);
    setup_talking_me_->setPublishPath("/3rdParty/TeamSpeak 3/Self Speaking");

    setup_channelname_ = new LH_Qt_QString(this, "Channel", "", LH_FLAG_HIDDEN | LH_FLAG_READONLY | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSINK);
    setup_channelname_->setPublishPath("/3rdParty/TeamSpeak 3/Channel Name");

    setup_connection_status_ = new LH_Qt_QStringList(this, "Connection Status", QStringList() << "Not Running" << "Not Connected" << "Connected", LH_FLAG_HIDDEN | LH_FLAG_READONLY | LH_FLAG_NOSAVE_DATA  | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSINK );
    setup_connection_status_->setPublishPath("/3rdParty/TeamSpeak 3/Connection Status");

    setup_microphone_status_ = new LH_Qt_QStringList(this, "Microphone Status", QStringList() << "N/A" << "None" << "Muted" << "Active",  LH_FLAG_READONLY | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSINK );
    setup_microphone_status_->setPublishPath("/3rdParty/TeamSpeak 3/Microphone Status");

    setup_speakers_status_   = new LH_Qt_QStringList(this, "Speaker Status"   , QStringList() << "N/A" << "None" << "Muted" << "Active",  LH_FLAG_READONLY | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSINK );
    setup_speakers_status_->setPublishPath("/3rdParty/TeamSpeak 3/Speaker Status");

    connect(setup_nickname_, SIGNAL(changed()), this, SLOT(updateMyDetails()));
    connect(socket_, SIGNAL(connected()), this, SLOT(TS3Connected()));
    connect(socket_, SIGNAL(disconnected()), this, SLOT(TS3Disconnected()));
    connect(socket_, SIGNAL(readyRead()), this, SLOT(TS3DataReceived()));
    connect(socket_, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(TS3ConnectionError(QAbstractSocket::SocketError)));

    updateStatus(false);
    updateTalking(true);
    openConnection();
    return 0;
}

int LH_QtPlugin_TS3::notify( int code, void *param )
{
    Q_UNUSED(param);
    if( code & LH_NOTE_SECOND && server_action_ == sa_disconnected  && tryConnectTimer_.elapsed() >= 5000)
        openConnection();
    return LH_NOTE_SECOND;
}

void LH_QtPlugin_TS3::openConnection()
{
    switch(socket_->state())
    {
    case QAbstractSocket::UnconnectedState:
#ifdef TS3_DEBUG_MESSAGES
        qDebug() << "LH_TS3: Attempting to connect.";
#endif
        server_action_ = sa_connecting;
        socket_->connectToHost("127.0.0.1", 25639);
        break;
    case QAbstractSocket::ConnectedState:
#ifdef TS3_DEBUG_MESSAGES
        qWarning() << "LH_TS3: Cannot connect - already connected.";
#endif
        server_action_ = sa_reconnect;
        socket_->disconnectFromHost();
        break;
    case QAbstractSocket::ClosingState:
#ifdef TS3_DEBUG_MESSAGES
        qWarning() << "LH_TS3: Cannot connect - still closing previous connection.";
#endif
        server_action_ = sa_reconnect;
        break;
    default:
#ifdef TS3_DEBUG_MESSAGES
        qWarning() << "LH_TS3: Cannot connect - previous connection attempt was in progress - aborting.";
#endif
        server_action_ = sa_reconnect;
        socket_->abort();
    }
}

void LH_QtPlugin_TS3::TS3Connected() {
#ifdef TS3_DEBUG_MESSAGES
    qDebug() << "LH_TS3: Connected";
#endif
    updateStatus(true);
    server_action_ = sa_eventregister_pending;
    sendMessage(QString("clientnotifyregister schandlerid=%1 event=notifytalkstatuschange \n"

                        "clientnotifyregister schandlerid=%1 event=notifyclientleftview \n"
                        "clientnotifyregister schandlerid=%1 event=notifycliententerview \n"
                        "clientnotifyregister schandlerid=%1 event=notifyclientupdated \n"
                        "clientnotifyregister schandlerid=%1 event=notifyclientmoved \n"

                        "clientnotifyregister schandlerid=%1 event=notifychannelcreated \n"
                        "clientnotifyregister schandlerid=%1 event=notifychanneledited \n"
                        "clientnotifyregister schandlerid=%1 event=notifychanneldeleted \n"
                        "clientnotifyregister schandlerid=%1 event=notifychannelmoved \n"
                        ).arg(0));
}

void LH_QtPlugin_TS3::TS3Disconnected()
{
#ifdef TS3_DEBUG_MESSAGES
    qDebug() << "LH_TS3: Disconnected";
#endif
    clients_.clear();
    channels_.clear();
    updateStatus(false);
    server_action_ = sa_disconnected;
    if(server_action_ == sa_reconnect)
        openConnection();
    else
        tryConnectTimer_.restart();
}

void LH_QtPlugin_TS3::TS3ConnectionError(QAbstractSocket::SocketError socketError)
{
    if(socketError == QAbstractSocket::ConnectionRefusedError)
    {
        server_action_ = sa_disconnected;
        tryConnectTimer_.restart();
#ifdef TS3_DEBUG_MESSAGES
        qDebug() << "LH_TS3: Connection Refused - will try again in a few moments";
#endif
    }
    else
    {
        qWarning() << "LH_TS3: Connection Error - " << socket_->errorString();
        server_action_ = sa_disconnected;
        clients_.clear();
        channels_.clear();
        updateStatus(false);
        tryConnectTimer_.restart();
    }
}

responseResult LH_QtPlugin_TS3::parseResult(QString msg)
{
    QRegExp rx("error id=(\\S+) msg=(\\S+)(?: extra_msg=(\\S+)){0,1}");
    if(rx.indexIn(msg)==-1)
        return responseResult();
    else
        return responseResult(true, (rx.cap(1)=="0"), (rx.cap(1).toInt()), rx.cap(2).replace("\\s"," "), rx.cap(3).replace("\\s"," "));
}

void LH_QtPlugin_TS3::TS3DataReceived()
{
    QString receivedMsg = QString::fromUtf8(socket_->readAll());
    //QRegExp rxSCHandler("selected schandlerid=([0-9]*)");
    QRegExp rxNotify("notify(\\w*) schandlerid=([0-9]*) (.*)");
    QRegExp rxMyID("clid=([0-9]*) cid=([0-9]*)");
    responseResult result = parseResult(receivedMsg);

    if(result.isResult && !result.isValid)
    {
        if(result.errorCode == 1794)
        {
            channels_.clear();
            clients_.clear();
            speakers_.clear();
            updateTalking();
#ifdef TS3_DEBUG_MESSAGES
            qDebug() << "LH_TS3: Client is not connected to a server.";
#endif
            updateStatus(true);
        }
        else
        {
            qWarning() << QString("LH_TS3: Unhandled Response Error: %1%2 (%3)").arg(result.errorMessage, (result.extraMessage==""?"":QString(" (%1)").arg(result.extraMessage))).arg(result.errorCode);
            // Q_ASSERT(false);
        }
    }
    else if(rxNotify.indexIn(receivedMsg)!=-1)
    {
        if(rxNotify.cap(1)=="talkstatuschange")
            talkChanged(rxNotify.cap(3));
        if(QString("clientleftview,cliententerview,clientupdated,clientmoved").split(',').contains(rxNotify.cap(1)) ||
           QString("channelcreated,channeledited,channeldeleted,channelmoved").split(',').contains(rxNotify.cap(1)) )
        {
#ifndef TS3_USER_DEFINED_UID
            request_WhoAmI();
#else
            request_ChannelList();
#endif
        }
    }
    else switch(server_action_)
    {
    case sa_eventregister_pending:
        if(!result.isResult)
            break;
#ifndef TS3_USER_DEFINED_UID
        request_WhoAmI();
#else
        request_ChannelList();
#endif
        break;
    case sa_whoami_pending:
        if(rxMyID.indexIn(receivedMsg)!=-1)
        {
            myclid_ = rxMyID.cap(1).toInt();
            updateMyDetails();
            request_ChannelList();
        }
        break;
    case sa_channellist_pending:
        if(channels_.load(receivedMsg))
        {
#ifdef TS3_USER_DEFINED_UID
            updateMyDetails();
#endif
            updateStatus(true,true,true);
#ifdef TS3_DEBUG_MESSAGES
            qDebug()<<"LH_TS3: Server has " << channels_.count() << " channel(s)";
#endif
            request_ClientList();
        }
        break;
    case sa_clientlist_pending:
        if(clients_.load(receivedMsg))
        {
            updateStatus(true,true,true,true);
            server_action_ = sa_listening;
            updateMyDetails();
        }
        break;
    case sa_none:
    case sa_disconnected:
    case sa_connecting:
    case sa_reconnect:
    case sa_listening:
        break;
    }
}

void LH_QtPlugin_TS3::request_WhoAmI()
{
    server_action_ = sa_whoami_pending;
    sendMessage("whoami");
}

void LH_QtPlugin_TS3::request_ChannelList()
{
    server_action_ = sa_channellist_pending;
    sendMessage("channellist");
}

void LH_QtPlugin_TS3::request_ClientList()
{
    server_action_ = sa_clientlist_pending;
    sendMessage("clientlist -voice");
}


int LH_QtPlugin_TS3::sendMessage(QString msg)
{
#ifdef TS3_DEBUG_MESSAGES
    qDebug() << QString(msg).toUtf8();
#endif
    return socket_->write(QString("%1\n").arg(msg).toUtf8());
}

void LH_QtPlugin_TS3::talkChanged(QString params)
{
    QRegExp rx("status=([0-9]*) isreceivedwhisper=([0-9]*) clid=([0-9]*)");

    bool status = false;
    // bool whisper = false;
    int clid = -1;

    if(rx.indexIn(params)!=-1)
    {
        status = (rx.cap(1).toInt()==1);
        // whisper = (rx.cap(2).toInt()==1);
        clid = rx.cap(3).toInt();
    }

    if(status)
    {
        if(speakers_.contains(clid))
            return;
        speakers_.insert(clid, clients_.value(clid));
    } else {
        if(!speakers_.contains(clid))
            return;
        speakers_.remove(clid);
    }
    updateTalking();
}

void LH_QtPlugin_TS3::updateTalking(bool force)
{
    QString talkingNames = speakers_.toString();
    if(force || setup_talking_->value()!=talkingNames)
    {
#ifdef TS3_DEBUG_MESSAGES
        qDebug() << "Talking: " << talkingNames;
#endif
        setup_talking_->setValue(talkingNames);
        setup_talking_details_->setValue(QString("<hr/><table style='margin-left:4px'>"
                                        "<tr><td><img src=':/images/%2.png'/></td><td width='56' style='padding-left:5px;'>Talking:<img src=':/images/sizer.png'/></td>   <td><img src=':/images/sizer.png'/>%1</td> </tr>"
                                        "</table>")
                                        .arg(talkingNames)
                                        .arg(talkingNames==""? "empty" : "talking")
                                        );
        setup_talking_me_->setValue(speakers_.contains(myclid_));
        emit talkingChanged(talkingNames);
    }
}

void LH_QtPlugin_TS3::updateStatus(bool isRunning, bool isConnected, bool showChannels, bool showClients)
{
    setup_connection_details_->setValue(QString("<table style='margin-left:4px'>"
                                   "<tr><td><img src=':/images/%5.png'/></td><td style='padding-left:5px'>TS3 is %1%2.</td></tr>"
                                   "</table>"
                                   "<hr/>"
                                   "<table style='margin-left:23px'>"
                                   "<tr><td><img src=':/images/sizer.png'/></td> <td width='56'>Channels:</td>  <td>%3</td></tr>"
                                   "<tr><td><img src=':/images/sizer.png'/></td> <td width='56'>Users:</td>     <td>%4</td></tr>"
                                   "</table>"
                                   )
            .arg(isRunning? "running" : "not running")
            .arg(isRunning? (isConnected? " and is connected" : " but is not connected") : "")
            .arg(showChannels? QString::number(channels_.count()) : "N/A")
            .arg(showClients? QString::number(clients_.count()) : "N/A")
            .arg(!isRunning? "notrunning" : (isConnected? "active" : "unconnected"))
            );
    setup_connection_status_->setValue(!isRunning? 0 : (!isConnected? 1 : 2));
    updateMyDetails();
}

void LH_QtPlugin_TS3::updateMyDetails()
{
#ifdef TS3_USER_DEFINED_UID
    myclid_ = clients_.findclid(setup_nickname_expression_->value());
#endif
    if(clients_.contains(myclid_))
    {
        clientdetail myClient = clients_.value(myclid_);
        channeldetail myChannel = channels_.value(myClient.cid);

        setup_user_detail_->setValue(QString("<hr/><table style='margin-left:23px'>"
                                            "<tr><td><img src=':/images/sizer.png'/></td> <td width='56'>Nickname:</td>   <td>%1</td></tr>"
                                            "<tr><td><img src=':/images/sizer.png'/></td> <td width='56'>Channel:</td>    <td>%2</td></tr>"
                                            "<tr><td><img src=':/images/sizer.png'/></td> <td width='56'>Microphone:</td> <td><img src=':/images/microphone%3.png'/></td></tr>"
                                            "<tr><td><img src=':/images/sizer.png'/></td> <td width='56'>Speakers:</td>   <td><img src=':/images/sound%4.png'/></td></tr>"
                                            "</table>")
                                    .arg(myClient.name)
                                    .arg(myChannel.name)
                                    .arg(!myClient.inputHardware? "-disabled" : (myClient.inputMuted? "-mute" : ""))
                                    .arg(!myClient.outputHardware? "-disabled" : (myClient.outputMuted? "-mute" : ""))
                                    );

        setup_microphone_status_->setValue(!myClient.inputHardware? 1 : (myClient.inputMuted? 2 : 3));
        setup_speakers_status_->setValue(!myClient.outputHardware? 1 : (myClient.outputMuted? 2 : 3));
        setup_channelname_->setValue(myChannel.name);
        setup_nickname_->setValue(myClient.name);
    } else {
        setup_user_detail_->setValue("");
        setup_microphone_status_->setValue(0);
        setup_speakers_status_->setValue(0);
        setup_channelname_->setValue("");
        setup_nickname_->setValue("");
    }
    emit myDetailsChanged();
}
