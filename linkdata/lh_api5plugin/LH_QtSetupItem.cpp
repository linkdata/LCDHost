/**
  \file     LH_QtSetupItem.cpp
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

#include "LH_QtSetupItem.h"
#include <QDebug>

LH_QtSetupItem::LH_QtSetupItem( LH_QtObject *parent, QString name, lh_setup_type type, int flags ) :
    QObject( parent ),
    order_(0)
{
    Q_ASSERT( parent != NULL );
    memset( &item_, 0, sizeof(item_) );
    setName(name);
    if( flags & LH_FLAG_FIRST ) order_ = -1;
    if( flags & LH_FLAG_LAST ) order_ = 1;
    item_.type = type;
    item_.flags = flags;
}

void LH_QtSetupItem::setName(QString s)
{
    QString newName = s.trimmed();
    if (newName.isEmpty())
      newName = QString("0x%1").arg((quintptr)this, QT_POINTER_SIZE * 2, 16, QChar('0'));
    if (s != newName) {
        qCritical( "<b>%s</b>: invalid setup item name: <tt>\"%s\"</tt>, using <tt>\"%s\"</tt>",
                  parent() ? parent()->metaObject()->className() : qPrintable(objectName()),
                  qPrintable(s), qPrintable(newName));
    }
    setObjectName(newName);
    name_array_ = newName.toUtf8();
    item_.name = name_array_.constData();
    return;
}

void LH_QtSetupItem::setup_change()
{
    emit changed();
    if( item_.flags & LH_FLAG_AUTORENDER ) parent()->requestRender();
    return;
}

void LH_QtSetupItem::setup_input( int flags, int value )
{
    emit input( QString(), flags, value );
    return;
}

void LH_QtSetupItem::refresh()
{
    // bool old_need = (item()->flags & LH_FLAG_NEEDREFRESH);
    if(parent()->callable())
    {
        /*
        if(old_need)
            qDebug() << "Setup item"
                     << parent()->metaObject()->className()
                     << parent()->objectName()
                     << objectName()
                     << "<strong>PROCESSING</strong>"
                        ;
                        */
        return parent()->callback(lh_cb_setup_refresh, item());
    }
    item()->flags |= LH_FLAG_NEEDREFRESH;
    /*
    qDebug() << "Setup item"
             << parent()->metaObject()->className()
             << parent()->objectName()
             << objectName()
             << (old_need ? "<strong>STILL WAITING</strong>" : "WAITING")
                ;
                */
    return;
}

void LH_QtSetupItem::setFlag( int f, bool state )
{
    if( state )
    {
        if( (item_.flags & f) == f ) return;
        item_.flags |= f;
    }
    else
    {
        if( !(item_.flags & f) ) return;
        item_.flags &= ~f;
    }
    refresh();
    return;
}


void LH_QtSetupItem::setOrder( int n )
{
    order_ = n;
}

void LH_QtSetupItem::setLink( QString s )
{
    setSubscribePath(s);
    return;
}

void LH_QtSetupItem::setMimeType( const char * s )
{
    item_.link.mime = s;
    refresh();
    return;
}

void LH_QtSetupItem::setPublishPath(const QString &s)
{
    publish_path_ = s.toUtf8();
    item_.link.publish = publish_path_.constData();
    refresh();
    return;
}

QString LH_QtSetupItem::publishPath() const
{
    return QString::fromUtf8(publish_path_);
}

void LH_QtSetupItem::setSubscribePath(const QString &s)
{
    bool is_publish = s.startsWith('@');
    bool need_fix = is_publish || s.startsWith('=');

    if(need_fix)
    {
        const QString new_path(s.trimmed().remove(0, 1));
        qWarning( "<b>%s</b>: old link style: <tt>%s</tt> FIXME!",
                  parent() ? parent()->metaObject()->className() : qPrintable(objectName()),
                  qPrintable(s));
        if(is_publish)
            setPublishPath(new_path);
        else
            setSubscribePath(new_path);
        return;
    }

    const QByteArray ba(s.toUtf8());
    memset(item_.link.subscribe, 0, sizeof(item_.link.subscribe));
    qstrncpy(item_.link.subscribe, ba.constData(), sizeof(item_.link.subscribe));
    refresh();
    return;
}

QString LH_QtSetupItem::subscribePath() const
{
    return QString::fromUtf8(item_.link.subscribe);
}

void LH_QtSetupItem::setHelp(QString s)
{
    if( s.isEmpty() ) help_.clear();
    else help_ = s.toUtf8();
    item_.help = help_.constData();
    return;
}

QString LH_QtSetupItem::help()
{
    return QString::fromUtf8(help_);
}
