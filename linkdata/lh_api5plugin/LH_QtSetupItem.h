/**
  \file     LH_QtSetupItem.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011, Johan Lindh

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

#ifndef LH_QTSETUPITEM_H
#define LH_QTSETUPITEM_H

#include <QObject>

#include "LH_QtObject.h"

class QColor;
class QFont;
class QFileInfo;
class QImage;

/**
  Helper class to manage setup items. Subclass this, mapping the LCDHost setup types to Qt classes.
  */
class LH_API5PLUGIN_EXPORT LH_QtSetupItem : public QObject
{
    Q_OBJECT

    int order_;
    QByteArray name_array_;
    QByteArray help_;
    QByteArray publish_path_;

protected:
    lh_setup_item item_;

public:
    explicit LH_QtSetupItem(LH_QtObject *parent, QString name, lh_setup_type type, int flags);

    LH_QtObject *parent() const { return static_cast<LH_QtObject *>(QObject::parent()); }

    virtual void setup_resize( size_t ) {}
    virtual void setup_change();
    virtual void setup_input( int flags, int value );

    void refresh();

    void setFlags( int f ) { if( item_.flags != f ) { item_.flags = f; refresh(); } }
    int flags() const { return item_.flags; }
    void setFlag( int f, bool state ); // set individual flag(s) on or off
    bool hasFlag( int f ) const { return (item_.flags & f) ? true : false; }

    void setName(QString s);
    QString name() const { return objectName(); }
    void setHelp(QString s);
    QString help();

    void setLink(QString s);
    QString link() { return subscribePath(); }

    void setPublishPath(const QString &s);
    QString publishPath() const;

    void setSubscribePath(const QString &s);
    QString subscribePath() const;

    void setMimeType( const char * s );
    const char *mimeType() const { return item_.link.mime; }

    lh_setup_item *item() { return &item_; }

    lh_setup_type type() { return item_.type; }

    int order() const { return order_; }
    void setOrder( int n );

    bool isVisible() const { return !isHidden(); }
    bool isHidden() const { return flags()&LH_FLAG_HIDDEN; }

signals:
    void changed();
    void change( bool );
    void change( int );
    void change( qreal );
    void change( QString );
    void input( QString, int, int );
    void set();

public slots:
    void setVisible( bool b ) { setFlag( LH_FLAG_HIDDEN, !b ); }
    void setHidden( bool b ) { setFlag( LH_FLAG_HIDDEN, b ); }
    void setReadonly( bool b ) { setFlag( LH_FLAG_READONLY, b ); }
    void setWriteable( bool b ) { setFlag( LH_FLAG_READONLY, !b ); }
    void setSaving( bool b ) { setFlag( LH_FLAG_NOSAVE_DATA|LH_FLAG_NOSAVE_LINK, !b ); }
    void setSavingData( bool b ) { setFlag( LH_FLAG_NOSAVE_DATA, !b ); }
    void setSavingLink( bool b ) { setFlag( LH_FLAG_NOSAVE_LINK, !b ); }

    virtual void setValue( bool ) { emit set(); }
    virtual void setValue( int ) { emit set(); }
    virtual void setValue( qreal ) { emit set(); }
    virtual void setValue( QString ) { emit set(); }
    virtual void setValue( const QColor & ) { emit set(); }
    virtual void setValue( const QFont & ) { emit set(); }
    virtual void setValue( const QFileInfo & ) { emit set(); }
    virtual void setValue( const QImage & ) { emit set(); }
};

#endif // LH_QTSETUPITEM_H
