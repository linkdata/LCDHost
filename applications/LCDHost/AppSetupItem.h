/*
  Copyright (c) 2009-2016 Johan Lindh <johan@linkdata.se>

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

#ifndef APPSETUPITEM_H
#define APPSETUPITEM_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>
#include <QMutex>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QVariant>
#include <QToolButton>
#include <QGridLayout>
#include <QMultiHash>

#include "lh_plugin.h"
#include "LibSetupItem.h"

class AppObject;

class AppSetupItem : public QObject
{
    Q_OBJECT

    // static QMultiHash<QString,AppSetupItem*> sinks_;
    // static QList<AppSetupItem*> sources_; // ASI's with the LH_FLAG_SOURCE set
    // static QList<AppSetupItem*> sinks_; // ASI's with non-blank 'source_'

    QVariant val_;

    // data linking
    QString mime_type_;
    QString publish_path_;
    QString subscribe_path_;

    lh_setup_type type_; // original type, or lh_type_none if not verified by LibInstance
    lh_setup_param param_; // plain copy of params, note param_.list is not safe
    unsigned flags_;
    QString extra_str_;
    int extra_int_;
    QString help_;
    QStringList paramlist_; // exploded param.list, if applicable
    bool verified_; // LibInstance verified this is still a valid value
    bool autopublished_;

    void changed( bool layoutmodified = true ); // changed by user using UI

    explicit AppSetupItem( QObject *parent ) :
        QObject(parent),
        val_(QVariant()),
        type_(lh_type_none),
        flags_(0),
        extra_str_(QString()),
        extra_int_(0),
        help_(QString()),
        verified_(false),
        autopublished_(false)
    {
        memset( &param_, 0, sizeof(param_) );
    }

public:
    static AppSetupItem *fromLibSetupItem( QObject *parent, LibSetupItem * );
    static AppSetupItem *fromSettings( QObject *parent, QSettings& settings, QString key );
    static AppSetupItem *fromXmlStream( QObject *parent, QXmlStreamReader & stream );
    static const char *typeName( int t );

    ~AppSetupItem();

    AppObject * parent() const
    {
        return reinterpret_cast<AppObject *>( QObject::parent() );
    }

    void autoPublish();

    void fixme( const char * s );

    QString name() const { return objectName(); }
    bool setObjectName(const QString &name);

    QVariant value() const { return val_; }
    lh_setup_type type() const { return type_; }
    unsigned flags() const { return flags_; }
    const lh_setup_param *param() const { return &param_; }
    QString extraStr() const { return extra_str_; }
    int extraInt() const { return extra_int_; }
    bool isVerified() const { return verified_; }
    bool hasHelp() const { return !help_.isEmpty(); }

    void setType( lh_setup_type t );

    QString path() const;
    const QString & mimeType() const { return mime_type_; }
    const QString & publishPath() const { return publish_path_; }
    const QString & subscribePath() const { return subscribe_path_; }
    void setMimeType( const QString & str = QString() );

    bool matchMime( const QString & mime ) const;
    QString publishPathUI() const;

    bool isSink() const { return !(flags_&LH_FLAG_NOSINK) && !subscribePath().isEmpty(); }
    bool isSource( const AppSetupItem * sink ) const;
    bool canSource( const AppSetupItem * sink ) const;

    // QStringRef linkPath() const { return link_.rightRef(link_.length()-1); }
    // bool isSource() const { return link_.startsWith('@'); }
    // bool isSink() const { return link_.startsWith('='); }

    bool isReadOnly() const { return flags_ & LH_FLAG_READONLY; }
    QString help() const { return help_; }
    bool isValid() const { return type_ != lh_type_none; }
    bool isHidden() const { return flags_ & LH_FLAG_HIDDEN; }
    bool isDataSaved() const { return !(flags_ & LH_FLAG_NOSAVE_DATA); }
    bool isLinkSaved() const { return !(flags_ & LH_FLAG_NOSAVE_LINK); }
    bool isSaved() const { return isDataSaved() || isLinkSaved(); }
    bool canStretch() const { return (type_ == lh_type_string_script) || (type_ == lh_type_integer_listbox); }
    QStringList paramList() const { return paramlist_; }

    void setFlags( unsigned f );

    QWidget *valueWidget( QWidget *parent = 0 );
    QWidget *buildUI( QGridLayout *grid, int row );


    bool isXmlSafe() const;

    void verified( LibSetupItem * ); // enumeration
    void fromLib( LibSetupItem * ); // data transfer
    void copy( const AppSetupItem& other );
    void informUI();
    void informLib();
    void pullSource();

    bool setFocus();
    const char* sourceIcon();

    bool load( QSettings& settings, QString key );
    void save( QSettings& settings ) const;

    bool load( QXmlStreamReader & stream );
    void save( QXmlStreamWriter & stream ) const;

    // static QStringList sources();
    bool setLinkPath(QString s);

public slots:
    void publish();
    void published( const AppSetupItem * asi );
    void textEdited( QString );
    void valueChanged( int );
    void valueChanged( double );
    void stateChanged( int );
    void stateChosen(QString,QString,int);
    void valueChosen(QString,QString,int);
    void showHelp(QVariant);
    void pressed();
    void released();
    void sourceDialog();
    void setPublishPath( const QString & path );
    void setSubscribePath( const QString & path );

signals:
    // void setupChange(AppSetupItem*);
    void requestPublishing();
    void publishing( const AppSetupItem * asi );
    void setEnabled( bool );
    void setVisible( bool );
    void setChecked( bool );
    void setText( QString );
    void setPixmap( const QPixmap& );
    void setValue( int );
    void setValue( double );
    void setBoldFont(bool);
    void setItalicFont(bool);
    void forwardFocus();
    void clear();
    void setItems( QVariant /* stringlist */, int /* index */ );
    void setMinMaxVal( int, int, int );
    void setMinMaxVal( double, double, double );
    void subscribePathChanged( const QString & );
    void publishPathChanged( const QString & );
    void sourceIconResourceChange(QString);
};

#endif // APPSETUPITEM_H
