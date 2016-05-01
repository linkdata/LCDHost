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

#include <QtDebug>
#include <QWidget>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QProgressBar>
#include <QDataStream>
#include <QStringList>
#include <QToolTip>
#include <QThread>

#include "LCDHost.h"
#include "AppScriptEditor.h"
#include "AppSetupItem.h"
#include "AppSetupFontInfo.h"
#include "AppSetupColorInfo.h"
#include "AppSetupFilename.h"
#include "AppSetupInputState.h"
#include "AppSetupInputValue.h"
#include "AppSetupComboBox.h"
#include "AppSetupListBox.h"
#include "AppSetupSpinBox.h"
#include "AppSetupButton.h"
#include "AppSetupSlider.h"
#include "AppSetupProgressBar.h"
#include "AppSetupDoubleSpinBox.h"
#include "AppRawInput.h"
#include "AppSourceDialog.h"
#include "MainWindow.h"
#include "LibSetupItem.h"
#include "AppSourceLinkButton.h"
#include "AppSetupItemHelp.h"
#include "AppLibrary.h"
#include "AppInstance.h"
#include "AppSetupLink.h"

#include "EventSetupItemRefresh.h"

// QMultiHash<QString,AppSetupItem*> AppSetupItem::sinks_;

AppSetupItem *AppSetupItem::fromLibSetupItem( QObject *parent, LibSetupItem *lsi )
{
    AppSetupItem *asi = new AppSetupItem( parent );
    if (asi->setObjectName(lsi->name())) {
        asi->verified_ = true;
        asi->fromLib(lsi);
        asi->autoPublish();
        return asi;
    }
    delete asi;
    return 0;
}

AppSetupItem *AppSetupItem::fromSettings( QObject *parent, QSettings& settings, QString key )
{
    if (key != key.trimmed()) {
        qWarning("AppSetupItem::fromSettings(): \"%s\": invalid key \"%s/%s\" in \"%s\"",
                 qPrintable(parent->objectName()),
                 qPrintable(settings.group()),
                 qPrintable(key),
                 qPrintable(settings.fileName())
                 );
        return 0;
    }

    AppSetupItem *asi = new AppSetupItem( parent );
    if( asi->load( settings, key ) )
    {
        asi->autoPublish();
        return asi;
    }
    delete asi;
    return NULL;
}

AppSetupItem *AppSetupItem::fromXmlStream( QObject *parent, QXmlStreamReader & stream )
{
    AppSetupItem *asi = new AppSetupItem( parent );
    if( asi->load( stream ) )
    {
        asi->autoPublish();
        return asi;
    }
    delete asi;
    return NULL;
}

AppSetupItem::~AppSetupItem()
{
    Q_ASSERT(!mainWindow || QThread::currentThread() == mainWindow->thread());
//    if( !subscribePath().isEmpty() )
//        sinks_.remove(subscribePath(),this);
//    Q_ASSERT( !sinks_.contains(subscribePath(),this) );
}

bool AppSetupItem::setObjectName(const QString &name)
{
    QObject::setObjectName(name);
    if (name.isEmpty() || name != name.trimmed()) {
        qCritical("AppSetupItem::setObjectName(\"%s\"): %s \"%s\": invalid object name",
                  qPrintable(name),
                  parent()->metaObject()->className(),
                  qPrintable(parent()->objectName())
                  );
        return false;
    }
    return true;
}

void AppSetupItem::fixme( const char * s )
{
    QString src;

    for( QObject * obj = parent(); obj; obj = obj->parent() )
    {
        if( AppLibrary * al = qobject_cast<AppLibrary *>(obj) )
        {
            src = al->objectName();
            break;
        }
        if( AppInstance * inst = qobject_cast<AppInstance*>( obj ) )
        {
            src = inst->classId();
            break;
        }
    }

    if( src.isEmpty() )
        src = objectName();

    qWarning( "<b>%s</b>: %s FIXME!", qPrintable(src), s );
}

void AppSetupItem::setFlags( unsigned f )
{
    if( f & LH_FLAG_NOSAVE )
    {
        f |= LH_FLAG_NOSAVE_DATA;
        if( ! (flags_ & LH_FLAG_NOSAVE_DATA) ) fixme("LH_FLAG_NOSAVE: => LH_FLAG_NOSAVE_DATA");
        if( f & LH_FLAG_HIDDEN )
        {
            f |= LH_FLAG_NOSAVE_LINK;
            if( ! (flags_ & LH_FLAG_NOSAVE_LINK) )
                fixme( "LH_FLAG_NOSAVE|LH_FLAG_HIDDEN: => LH_FLAG_NOSAVE_LINK" );
        }
    }
    flags_ = f;
}

void AppSetupItem::setType( lh_setup_type t )
{
    type_ = t;
    if( mimeType().isEmpty() )
        setMimeType( typeName(t) );
    return;
}

QString AppSetupItem::path() const
{
    if( parent() == 0 || !isVerified() || objectName().isEmpty() ) return QString();
    return parent()->path().append('/').append( objectName() );
}

bool AppSetupItem::matchMime( const QString & mime ) const
{
    int max = mimeType().size();
    if( max > mime.size() ) max = mime.size();
    for( int i = 0; i < max; ++ i )
    {
        if( mimeType().at(i) != mime.at(i) )
        {
            if( mime.at(i) == '*' || mimeType().at(i) == '*' ) return true;
            return false;
        }
        if( mime.at(i) == ';' ) return true;
    }
    return true;
}

QString AppSetupItem::publishPathUI() const
{
    if( (flags() & LH_FLAG_NOSOURCE) ||
            !isVerified() ||
            objectName().isEmpty() ||
            parent() == 0 ) return QString();
    if( publishPath().startsWith('/') ) return publishPath();
    return QString( parent()->path() ).append('/').append( objectName() );
}

bool AppSetupItem::isSource( const AppSetupItem * sink ) const
{
    if( sink == 0 || sink == this ) return false;
    if( !isVerified() || sink->subscribePath().isEmpty() ) return false;

    return (sink->subscribePath() == publishPath());// || (sink->subscribePath() == publishPathAuto()) ;
}

bool AppSetupItem::canSource( const AppSetupItem * sink ) const
{
    if( sink == 0 || sink == this ) return false;
    return isVerified() && matchMime( sink->mimeType() );
}

void AppSetupItem::setMimeType( const QString & str )
{
    if( str.isEmpty() ) mime_type_ = typeName(type());
    else mime_type_ = str.simplified();
}

void AppSetupItem::autoPublish()
{
    if(!(flags_ & LH_FLAG_NOSOURCE) && publish_path_.isEmpty())// && subscribe_path_.isEmpty())
    {
        // candidate for autopublish
        if(parent() && !parent()->objectName().isEmpty() && !objectName().isEmpty())
        {
            QString autopath(parent()->objectName().append('/').append(objectName()));
            // qDebug() << "AppSetupItem::autoPublish()" << path() << "as" << autopath;
            setPublishPath(autopath);
            autopublished_ = true;
        }
    }
}

bool AppSetupItem::setLinkPath(QString path)
{
    bool is_publish = path.startsWith('@');
    bool need_fix = is_publish || path.startsWith('=');

    if(need_fix)
    {
        fixme(QByteArray("old link style: <tt>" ).append(path).append("</tt>").constData() );
        path.remove(0, 1);
        if(is_publish)
            setPublishPath(path.trimmed());
        else
            setSubscribePath(path.trimmed());
    }

    return need_fix;
}

void AppSetupItem::setPublishPath( const QString & path )
{
    Q_ASSERT(path == path.trimmed());

    if(path != path.trimmed())
    {
        qWarning() << "AppSetupItem::setPublishPath()" << this->path() << "whitespace in linkpath" << path;
        return setPublishPath(path.trimmed());
    }

    if(path.isEmpty() && autopublished_)
        return;

    if( publish_path_ != path )
    {
        autopublished_ = false;
        AppSetupLink *link = AppSetupLink::factory(publish_path_);
        if(link)
        {
            disconnect(link, &AppSetupLink::requestPublishing, this, &AppSetupItem::publish);
            disconnect(this, &AppSetupItem::publishing, link, &AppSetupLink::publishing);
        }
        publish_path_ = path.trimmed();
        link = AppSetupLink::factory(publish_path_);
        if(link)
        {
            connect(this, &AppSetupItem::publishing, link, &AppSetupLink::publishing);
            connect(link, &AppSetupLink::requestPublishing, this, &AppSetupItem::publish);
            publish();
        }
        emit publishPathChanged( publish_path_ );
        // qDebug() << "AppSetupItem::setPublishPath" << publishPath();
    }
    return;
}

void AppSetupItem::setSubscribePath( const QString & path )
{
    Q_ASSERT(path == path.trimmed());

    if(subscribe_path_ != path)
    {
        AppSetupLink *link = AppSetupLink::factory(subscribe_path_);
        if(link)
        {
            disconnect(link, &AppSetupLink::publishing, this, &AppSetupItem::published);
            disconnect(this, &AppSetupItem::requestPublishing, link, &AppSetupLink::requestPublishing);
        }
        // sinks_.remove(subscribe_path_,this);
        // Q_ASSERT( sinks_.count(subscribe_path_,this) == 0 );
        subscribe_path_ = path.trimmed();
        link = AppSetupLink::factory(subscribe_path_);
        if(link)
        {
            connect(this, &AppSetupItem::requestPublishing, link, &AppSetupLink::requestPublishing);
            connect(link, &AppSetupLink::publishing, this, &AppSetupItem::published);
            emit requestPublishing();
        }
        emit sourceIconResourceChange( sourceIcon() );
        emit subscribePathChanged( subscribePath() );
        // qDebug() << "AppSetupItem::setSubscribePath" <<  subscribePath();
    }

    // Q_ASSERT( sinks_.contains(subscribePath(),this) == !subscribePath().isEmpty() );
    return;
}

void AppSetupItem::publish()
{
    if(!publish_path_.isEmpty())
    {
        // if (type_ == lh_type_integer_color) qDebug() << "AppSetupItem::publish()" << path() << publish_path_;
        emit publishing(this);
    }
    /*
    foreach( AppSetupItem * sink, sinks_.values(publishPath()) )
        sink->copy( *this );
    foreach( AppSetupItem * sink, sinks_.values(publishPathAuto()) )
        sink->copy( *this );
        */
}

void AppSetupItem::published(const AppSetupItem * asi)
{
    Q_ASSERT(subscribePath() == asi->publishPath());// || subscribePath() == asi->publishPathAuto());
    if(!subscribe_path_.isEmpty())
    {
        // qDebug() << "AppSetupItem::published()" << path() << subscribe_path_<< "from"<< asi->path();
        copy(*asi);
    }
}

QWidget *AppSetupItem::buildUI( QGridLayout *grid, int row )
{
    QWidget *retv = NULL;
    QLabel *title = 0;
    QWidget *value = 0;
    AppSourceLinkButton *linky = 0;
    AppSetupItemHelp *helpicon = 0;
    int firstcolumn = 0;

    // Column layout:
    // [link] [help] [title] [value]

    if( !(flags()&LH_FLAG_NOSINK) )
    {
        linky = new AppSourceLinkButton( grid->parentWidget() );
        linky->setIconResource( sourceIcon() );
        if( isSink() ) linky->setToolTip( QString("Subscribing to '%1'").arg(subscribePath()) );
        connect( this, SIGNAL(sourceIconResourceChange(QString)), linky, SLOT(setIconResource(QString)), Qt::UniqueConnection );
        connect( linky, SIGNAL(clicked()), this, SLOT(sourceDialog()), Qt::UniqueConnection );
        connect( this, SIGNAL(setVisible(bool)), linky, SLOT(setVisible(bool)), Qt::UniqueConnection );
        grid->addWidget( linky, row, 0, 1, 1 );
        firstcolumn = 1;
    }

    if( hasHelp() )
    {
        helpicon = new AppSetupItemHelp( help(), grid->parentWidget() );
        connect( this, SIGNAL(setVisible(bool)), helpicon, SLOT(setVisible(bool)), Qt::UniqueConnection );
        grid->addWidget( helpicon, row, 1, 1, 1 );
        firstcolumn = 2;
    }

    if( name().startsWith('^') || flags()&LH_FLAG_BLANKTITLE )
        title = new QLabel("", grid->parentWidget());
    else
        title = new QLabel( name(), grid->parentWidget() );
    title->setOpenExternalLinks(true);

    value = valueWidget( grid->parentWidget() );
    if( canStretch() ) value->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    else value->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );

    if( flags()&LH_FLAG_HIDETITLE || name().startsWith('~') )
    {
        title->hide();
        grid->addWidget( value, row, firstcolumn, 1, -1 );
        connect( this, SIGNAL(setEnabled(bool)), value, SLOT(setEnabled(bool)), Qt::UniqueConnection );
        connect( this, SIGNAL(setVisible(bool)), value, SLOT(setVisible(bool)), Qt::UniqueConnection );
    }
    else if( flags()&LH_FLAG_HIDEVALUE )
    {
        value->hide();
        grid->addWidget( title, row, 2, 1, -1 );
        connect( this, SIGNAL(setVisible(bool)), title, SLOT(setVisible(bool)), Qt::UniqueConnection );
    }
    else
    {
        grid->addWidget( title, row, 2, 1, 1, Qt::AlignRight );
        grid->addWidget( value, row, 3, 1, 1 );
        connect( this, SIGNAL(setVisible(bool)), title, SLOT(setVisible(bool)), Qt::UniqueConnection );
        connect( this, SIGNAL(setEnabled(bool)), value, SLOT(setEnabled(bool)), Qt::UniqueConnection );
        connect( this, SIGNAL(setVisible(bool)), value, SLOT(setVisible(bool)), Qt::UniqueConnection );
    }

    if( canStretch() ) value->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    else value->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );

    informUI();

    return retv;
}

QWidget *AppSetupItem::valueWidget( QWidget *parent )
{
    QWidget *retv = NULL;

    switch( type() )
    {
    case lh_type_none:
        return retv;
    case lh_type_integer:
        {
            AppSetupSpinBox *w = new AppSetupSpinBox(parent);
            connect( w, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)) );
            connect( this, SIGNAL(setValue(int)), w, SLOT(setValue(int)) );
            connect( this, SIGNAL(forwardFocus()), w, SLOT(setFocus()) );
            connect( this, SIGNAL(setMinMaxVal(int,int,int)), w, SLOT(setMinMaxVal(int,int,int)) );
            retv = w;
        }
        break;
    case lh_type_integer_color:
        {
            AppSetupColorInfo *w = new AppSetupColorInfo(parent);
            connect( w, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)) );
            connect( this, SIGNAL(setValue(int)), w, SLOT(setValue(int)) );
            retv = w;
        }
        break;
    case lh_type_integer_boolean:
        {
            QCheckBox *w;
            if( objectName().startsWith('^') )
            {
                QString cbtext = objectName();
                w = new QCheckBox(cbtext.remove(0,1), parent);
            }
            else w = new QCheckBox(parent);
            connect( w, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)) );
            connect( this, SIGNAL(setChecked(bool)), w, SLOT(setChecked(bool)) );
            retv = w;
        }
        break;
    case lh_type_integer_slider:
        {
            AppSetupSlider *w = new AppSetupSlider( Qt::Horizontal, parent );
            connect( w, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)) );
            connect( this, SIGNAL(setValue(int)), w, SLOT(setValue(int)) );
            connect( this, SIGNAL(setMinMaxVal(int,int,int)), w, SLOT(setMinMaxVal(int,int,int)) );
            retv = w;
        }
        break;
    case lh_type_integer_progress:
        {
            AppSetupProgressBar *w = new AppSetupProgressBar( parent );
            connect( w, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)) );
            connect( this, SIGNAL(setValue(int)), w, SLOT(setValue(int)) );
            connect( this, SIGNAL(setMinMaxVal(int,int,int)), w, SLOT(setMinMaxVal(int,int,int)) );
            retv = w;
        }
        break;
    case lh_type_integer_list:
        {
            AppSetupComboBox *w = new AppSetupComboBox(parent);
            connect( w, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged(int)) );
            connect( this, SIGNAL(setValue(int)), w, SLOT(setCurrentIndex(int)) );
            connect( this, SIGNAL(clear()), w, SLOT(clear()) );
            connect( this, SIGNAL(setItems(QVariant,int)), w, SLOT(setItems(QVariant,int)) );
            retv = w;
        }
        break;
    case lh_type_integer_listbox:
        {
            AppSetupListBox *w = new AppSetupListBox(parent);
            connect( w, SIGNAL(currentRowChanged(int)), this, SLOT(valueChanged(int)) );
            connect( this, SIGNAL(setValue(int)), w, SLOT(setCurrentRow(int)) );
            connect( this, SIGNAL(clear()), w, SLOT(clear()) );
            connect( this, SIGNAL(setItems(QVariant,int)), w, SLOT(setItems(QVariant,int)) );
            retv = w;
        }
        break;
        break;
    case lh_type_fraction:
        {
            AppSetupDoubleSpinBox *w = new AppSetupDoubleSpinBox(parent);
            connect( w, SIGNAL(valueChanged(double)), this, SLOT(valueChanged(double)) );
            connect( this, SIGNAL(setValue(double)), w, SLOT(setValue(double)) );
            connect( this, SIGNAL(setMinMaxVal(double,double,double)), w, SLOT(setMinMaxVal(double,double,double)) );
            retv = w;
        }
        break;
    case lh_type_string_font:
        {
            AppSetupFontInfo *w = new AppSetupFontInfo(parent);
            connect( w, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)) );
            connect( this, SIGNAL(setText(QString)), w, SLOT(setText(QString)) );
            retv = w;
        }
        break;
    case lh_type_string:
        {
            QLineEdit *w = new QLineEdit(parent);
            connect( w, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)) );
            connect( this, SIGNAL(setText(QString)), w, SLOT(setText(QString)) );
            retv = w;
        }
        break;
    case lh_type_string_html:
        {
            QLabel *w = new QLabel(parent);
            w->setTextFormat( Qt::RichText );
            connect( this, SIGNAL(setText(QString)), w, SLOT(setText(QString)) );
            retv = w;
        }
        break;
    case lh_type_string_button:
        {
            AppSetupButton *w = new AppSetupButton(parent);
            connect( w, SIGNAL(pressed()), this, SLOT(pressed()) );
            connect( w, SIGNAL(released()), this, SLOT(released()) );
            connect( this, SIGNAL(setText(QString)), w, SLOT(setButtonText(QString)) );
            retv = w;
        }
        break;
    case lh_type_string_script:
        {
            QWidget *w = new QWidget(parent);
            QVBoxLayout *layout = new QVBoxLayout(w);
            QToolButton *tb = new QToolButton();
            tb->setText(tr("Done editing"));
            tb->setEnabled(false);
            // QPlainTextEdit *editor = new QPlainTextEdit(w);
            AppScriptEditor *editor = new AppScriptEditor(w);
            connect( editor, SIGNAL(contentUnstable(bool)), tb, SLOT(setEnabled(bool)) );
            connect( editor, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)) );
            connect( this, SIGNAL(setText(QString)), editor, SLOT(setText(QString)) );
            //connect( this, SIGNAL(setText(QString)), editor, SLOT(setPlainText(QString)) );
            layout->addWidget( editor, 1 );
            layout->addWidget( tb, 0, Qt::AlignRight );
            retv = w;
        }
        break;
    case lh_type_string_filename:
        {
            AppSetupFilename *w = new AppSetupFilename(parent);
            connect( w, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)) );
            connect( this, SIGNAL(setText(QString)), w, SLOT(setText(QString)) );
            retv = w;
        }
        break;
    case lh_type_string_inputstate:
        {
            AppSetupInputState *w = new AppSetupInputState(parent);
            connect( w, SIGNAL(inputChosen(QString,QString,int)), this, SLOT(stateChosen(QString,QString,int)) );
            connect( this, SIGNAL(setText(QString)), w, SLOT(setText(QString)) );
            retv = w;
        }
        break;
    case lh_type_string_inputvalue:
        {
            AppSetupInputValue *w = new AppSetupInputValue(parent);
            connect( w, SIGNAL(inputChosen(QString,QString,int)), this, SLOT(valueChosen(QString,QString,int)) );
            connect( this, SIGNAL(setText(QString)), w, SLOT(setText(QString)) );
            retv = w;
        }
        break;
    case lh_type_image_png:
    case lh_type_image_qimage:
        {
            retv = new QWidget(parent);
            QHBoxLayout *toplayout = new QHBoxLayout(retv);
            QGroupBox *groupbox = new QGroupBox(retv);
            toplayout->setMargin(0);
            toplayout->addWidget( groupbox, 0 );
            toplayout->addWidget( new QWidget(), 1 );
            QHBoxLayout *layout = new QHBoxLayout(groupbox);
            layout->setSizeConstraint(QLayout::SetFixedSize);
            QLabel *label = new QLabel(groupbox);
            layout->addWidget( label );
            connect( this, SIGNAL(setPixmap(QPixmap)), label, SLOT(setPixmap(QPixmap)) );
        }
        break;
    case lh_type_last:
#ifdef QT_NO_DEBUG
    default:
#endif
        qWarning() << "Unhandled data type" << (int)type() << "for AppSetupItem" << objectName();
        Q_ASSERT(0);
        break;
    }

    return retv;
}

void AppSetupItem::changed( bool layoutmodified )
{
    Q_ASSERT( QThread::currentThread() == mainWindow->thread() );

    if( layoutmodified )
    {
        if( isSaved() )
        {
            if( parent()->inherits("AppInstance") )
                ::layoutModified();
        }
    }

    publish();
    informLib();
    return;
}

void AppSetupItem::stateChanged( int state )
{
    if(!val_.isValid() || val_.toBool() != (state == Qt::Checked) )
    {
        val_ = ( state == Qt::Checked );
        changed();
    }
    return;
}

void AppSetupItem::textEdited( QString s )
{
    if(!val_.isValid() || val_.toString() != s )
    {
        val_ = s;
        changed();
    }
    return;
}

void AppSetupItem::valueChanged( int i )
{
    if(!val_.isValid() || i != val_.toInt())
    {
        val_ = i;
        changed();
    }
    return;
}

void AppSetupItem::valueChanged( double d )
{
    if(!val_.isValid() || d != val_.toDouble())
    {
        val_ = d;
        changed();
    }
    return;
}

void AppSetupItem::informUI()
{
    emit setEnabled( !(flags() & (LH_FLAG_READONLY|LH_FLAG_HIDDEN)) );
    emit setVisible( !(flags() & LH_FLAG_HIDDEN) );

    switch( type() )
    {
    case lh_type_none:
    case lh_type_last:
        break;
    case lh_type_integer_color:
        emit setValue( val_.toInt() );
        break;
    case lh_type_integer:
    case lh_type_integer_slider:
    case lh_type_integer_progress:
        emit setMinMaxVal( param_.slider.min, param_.slider.max, val_.toInt() );
        break;
    case lh_type_integer_list:
    case lh_type_integer_listbox:
        Q_ASSERT( param_.list == NULL );
        emit setItems( paramlist_, val_.toInt() );
        return;
    case lh_type_integer_boolean:
        emit setChecked( val_.toBool() );
        break;
    case lh_type_fraction:
        emit setMinMaxVal( param_.range.min, param_.range.max, val_.toDouble() );
        break;
    case lh_type_string:
    case lh_type_string_html:
    case lh_type_string_button:
    case lh_type_string_script:
    case lh_type_string_filename:
    case lh_type_string_inputstate:
    case lh_type_string_inputvalue:
    case lh_type_string_font:
        emit setText( val_.toString() );
        break;
    case lh_type_image_png:
    case lh_type_image_qimage:
        emit setPixmap( QPixmap::fromImage( val_.value<QImage>() ) );
        break;
    }
}

void AppSetupItem::informLib()
{
    if( AppObject *p = qobject_cast<AppObject*>(parent()) )
    {
        p->id().postLibEvent( new EventSetupItemRefresh( this ) );
    }
    else
    {
        qCritical() << "Setup item"
                    << metaObject()->className() << objectName()
                    << "parent not AppObject:"
                    << ( parent() ? parent()->metaObject()->className() : "NULL")
                       ;
    }
}

void AppSetupItem::verified( LibSetupItem *lsi )
{
    verified_ = true;
    setType( lsi->type() );
    help_ = lsi->help();
    paramlist_ = lsi->paramList();
    memcpy( &param_, lsi->param(), sizeof(param_) );
    setFlags(lsi->flags());

    if( !isDataSaved() )
    {
        setObjectName( lsi->name() );
        val_ = lsi->value();
    }

    autoPublish();
    if(!isLinkSaved())
    {
        if(!setLinkPath(lsi->subscribe()))
        {
            setSubscribePath(lsi->subscribe());
            setPublishPath(lsi->publish());
        }
    }
}

void AppSetupItem::fromLib( LibSetupItem *lsi )
{
    if (!setObjectName(lsi->name()))
        return;
    help_ = lsi->help();
    paramlist_ = lsi->paramList();
    memcpy( &param_, lsi->param(), sizeof(param_) );
    setFlags(lsi->flags());
    setType(lsi->type());
    val_ = lsi->value();
    setMimeType(lsi->mimetype());
    autoPublish();
    if(!isLinkSaved())
    {
        if(!setLinkPath(lsi->subscribe()))
        {
            setSubscribePath(lsi->subscribe());
            setPublishPath(lsi->publish());
        }
    }
    // pullSource();
    informUI();
    publish();
}

bool AppSetupItem::setFocus()
{
    if( ! (flags() & LH_FLAG_FOCUS) ) return false;
    if( flags() & (LH_FLAG_HIDDEN|LH_FLAG_READONLY) ) return false;
    emit forwardFocus();
    return true;
}

void AppSetupItem::stateChosen(QString desc,QString control,int flags)
{
    if( control != extra_str_ || flags != extra_int_ )
    {
        val_ = desc;
        extra_str_ = control;
        extra_int_ = flags;
        changed();
    }
    return;
}

void AppSetupItem::valueChosen(QString desc,QString control,int flags)
{
    Q_UNUSED(flags);
    if( control != extra_str_ || 0 != extra_int_ )
    {
        val_ = desc;
        extra_str_ = control;
        extra_int_ = 0;
        changed();
    }
    return;
}

void AppSetupItem::showHelp( QVariant pt )
{
    QToolTip::showText( pt.toPoint(), help() );
}

void AppSetupItem::pressed()
{
    extra_int_ = 1;
    changed(false);
}

void AppSetupItem::released()
{
    extra_int_ = 0;
    changed(false);
}


bool AppSetupItem::isXmlSafe() const
{
    QVariant value = val_;
    if( !value.canConvert<QString>() ) return false;
    if( !value.convert( QVariant::String ) ) return false;
    QString str = value.toString();
    for( int i = 0; i < str.size(); ++i )
    {
        QChar ch = str[i];
        if( ch < QChar(' ') && ch != QChar(0x09) && ch != QChar(0x0A) && ch != QChar(0x0D) ) return false;
    }
    return true;
}

bool AppSetupItem::load( QSettings& settings, QString key )
{
    if (!setObjectName(key))
        return false;
    settings.beginGroup(key);
    setType((lh_setup_type) settings.value("type").toInt());
    val_.setValue( settings.value("value") );
    extra_str_ = settings.value("extrastr").toString();
    extra_int_ = settings.value("extraint").toInt();
    if(settings.contains("link"))
        setLinkPath(settings.value("link").toString());
    if(settings.contains("subscribe"))
        setSubscribePath(settings.value("subscribe").toString());
    if(settings.contains("publish"))
        setPublishPath(settings.value("publish").toString());
    settings.endGroup();
    // pullSource();
    // publish();
    return true;
}

void AppSetupItem::save( QSettings& settings ) const
{
    if( !isValid() ) return;
    settings.beginGroup( objectName() );
    settings.setValue("type", (int)type() );
    if( isDataSaved() ) settings.setValue("value",value());
    if( isLinkSaved() )
    {
        settings.setValue("subscribe",subscribePath());
        settings.setValue("publish",publishPath());
    }
    settings.setValue("extrastr",extra_str_);
    settings.setValue("extraint",extra_int_);
    settings.endGroup();
}

bool AppSetupItem::load( QXmlStreamReader & stream )
{
    int startline = stream.lineNumber();
    if( stream.name() != "setupitem" )
    {
        qDebug() << "AppSetupItem::load(): expected 'setupitem' in xml";
        return false;
    }

    while( !stream.atEnd() )
    {
        stream.readNext();

        if( stream.isStartElement() )
        {
            if( stream.name() == "identity" )
            {
                setObjectName( stream.attributes().value("name").toString() );
                QString typestr(stream.attributes().value("type").toString());
                setType( (lh_setup_type) typestr.toInt() );
            }
            if( stream.name() == "link" )
            {
                QString linkpath(stream.readElementText());
                if(!setLinkPath(linkpath))
                    setSubscribePath(linkpath);
            }
            if( stream.name() == "subscribe" )
            {
                setSubscribePath( stream.readElementText() );
            }
            if( stream.name() == "publish" )
            {
                setPublishPath( stream.readElementText() );
            }
            if( stream.name() == "value" )
            {
                QVariant::Type vartype = QVariant::nameToType( stream.attributes().value("type").toString().toLatin1().constData() );
                if( vartype != QVariant::Invalid ) val_ = QVariant( vartype );
                val_.setValue( stream.readElementText() );
                if( stream.error() )
                    qDebug() << "AppSetupItem::load(): invalid value for" << objectName() << "at line" << stream.lineNumber();
            }
            if( stream.name() == "base64" )
            {
                QString stored_type = stream.attributes().value("type").toString();
                QByteArray array = QByteArray::fromBase64( stream.readElementText().toLatin1() );
                QDataStream ds( &array, QIODevice::ReadWrite );
                ds >> val_;
                if( !val_.isValid() || stream.error() || stored_type != val_.typeName() )
                    qDebug() << "AppSetupItem::load(): invalid data for" << objectName() <<
                            (val_.typeName()?val_.typeName():"<untyped>") << "at line" << stream.lineNumber();
            }
            if( stream.name() == "extra" )
            {
                extra_str_ = stream.attributes().value("str").toString();
                extra_int_ = stream.attributes().value("int").toString().toInt();
            }
        }

        if( stream.isEndElement() )
        {
            if( stream.name() == "setupitem" )
            {
                if( !isValid() ) qDebug() << "AppSetupItem::load(): invalid item line" << stream.lineNumber();

                stream.readNext();

                if( lh_log_load )
                    qDebug() << QString("<span style=\"background-color: #f0f0f0;\">%1</span> <tt>&nbsp;&nbsp;%2 = %3</tt>")
                        .arg(startline,5,10,QChar('0'))
                        .arg(name())
                        .arg(value().toString());

                pullSource();
                publish();

                return true;
            }
        }
    }

    qDebug() << "AppSetupItem::load(): xml stream ended prematurely:" << objectName() << "lines" << startline << "to" << stream.lineNumber();
    return false;
}

void AppSetupItem::save( QXmlStreamWriter & stream ) const
{
    if( !isSaved() ) return;

    if( isValid() )
    {
        stream.writeStartElement("setupitem");
        stream.writeEmptyElement("identity");
        stream.writeAttribute("name",objectName());
        stream.writeAttribute("type",QString::number((int)type()));
        if( isLinkSaved() )
        {
            if( !subscribePath().isEmpty() )
            {
                stream.writeStartElement("subscribe");
                stream.writeCharacters(subscribePath());
                stream.writeEndElement();
            }
            if( !publishPath().isEmpty() )
            {
                stream.writeStartElement("publish");
                stream.writeCharacters(publishPath());
                stream.writeEndElement();
            }
        }
        if( isDataSaved() && val_.isValid() )
        {
            if( isXmlSafe() )
            {
                stream.writeStartElement("value");
                stream.writeAttribute( "type", val_.typeName() );
                stream.writeCharacters( val_.toString() );
                stream.writeEndElement();
            }
            else
            {
                QByteArray array;
                QDataStream ds( &array, QIODevice::ReadWrite );
                ds << val_;
                stream.writeStartElement("base64");
                stream.writeAttribute( "type", val_.typeName() );
                stream.writeCharacters( QString::fromLatin1(array.toBase64()) );
                stream.writeEndElement();
            }
        }
        if( !extra_str_.isEmpty() || extra_int_ != 0 )
        {
            stream.writeEmptyElement("extra");
            stream.writeAttribute("str",extra_str_);
            stream.writeAttribute("int",QString::number(extra_int_));
        }
        stream.writeEndElement();
    }
}

const char* AppSetupItem::sourceIcon()
{
    if(mainWindow && isSink())
    {
        foreach (AppLibrary* app_lib, mainWindow->plugins()) {
            foreach( AppSetupItem * asi, app_lib->findChildren<AppSetupItem *>() ) {
                if( asi->isSource(this) )
                    return ":/lcdhost/images/sourcegreen.png";
            }
        }
        return ":/lcdhost/images/sourcered.png";
    }
    return ":/lcdhost/images/source.png";
}

void AppSetupItem::sourceDialog()
{
    AppSourceDialog *dlg = new AppSourceDialog( this, mainWindow );
    dlg->exec();
}

const char * AppSetupItem::typeName( int t )
{
    switch(t)
    {
    case lh_type_none: return "application/x-type-none";
    case lh_type_integer: return "text/x-number; ui=integer";
    case lh_type_integer_boolean: return "text/x-boolean";
    case lh_type_integer_color: return "application/x-color";
    case lh_type_integer_slider: return "text/x-number; ui=slider";
    case lh_type_integer_progress: return "text/x-number; ui=progress";
    case lh_type_integer_list: return "text/x-number; ui=list";
    case lh_type_fraction: return "text/x-number; ui=fraction";
    case lh_type_string: return "text/*";
    case lh_type_string_script: return "text/plain; ui=script";
    case lh_type_string_filename: return "text/plain; ui=filename";
    case lh_type_string_font: return "application/x-font";
    case lh_type_string_inputstate: return "application/x-inputstate";
    case lh_type_string_inputvalue: return "application/x-inputvalue";
    case lh_type_image_png: return "image/png";
    case lh_type_image_qimage: return "image/x-argb32";
    case lh_type_integer_listbox: return "text/x-number; ui=listbox";
    case lh_type_string_button: return "text/plain; ui=button";
    case lh_type_string_html: return "text/html";
    case lh_type_last: return "application/x-type-last";
    }
    return "application/x-type-invalid";
}

void AppSetupItem::pullSource()
{
    emit requestPublishing();
    return;
}

void AppSetupItem::copy( const AppSetupItem & other )
{
    if( this == &other || ! other.isValid() ) return;
    switch( type() )
    {
    case lh_type_none:
    case lh_type_last:
        return;
    case lh_type_integer_color:
        val_ = other.value().toInt();
        break;
    case lh_type_integer:
    case lh_type_integer_slider:
    case lh_type_integer_progress:
        val_ = other.value().toInt();
        if( other.type() == lh_type_integer ||
                other.type() == lh_type_integer_slider ||
                other.type() == lh_type_integer_progress
                )
        {
            param_.slider.min = other.param()->slider.min;
            param_.slider.max = other.param()->slider.max;
        }
        break;
    case lh_type_integer_list:
    case lh_type_integer_listbox:
        val_ = other.value().toInt();
        if( other.type() == lh_type_integer_list ||
                other.type() == lh_type_integer_listbox )
        {
            paramlist_ = other.paramlist_;
        }
        break;
    case lh_type_integer_boolean:
        val_ = other.value().toBool();
        break;
    case lh_type_fraction:
        val_ = other.value().toDouble();
        if( other.type() == lh_type_fraction )
        {
            param_.range.min = other.param()->range.min;
            param_.range.max = other.param()->range.max;
        }
        break;
    case lh_type_string:
    case lh_type_string_html:
    case lh_type_string_button:
    case lh_type_string_script:
    case lh_type_string_filename:
    case lh_type_string_inputstate:
    case lh_type_string_inputvalue:
    case lh_type_string_font:
        val_ = other.value().toString();
        break;
    case lh_type_image_png:
    case lh_type_image_qimage:
        if( other.type() == lh_type_image_png || other.type() == lh_type_image_qimage )
        {
            val_ = other.value();
        }
        break;
    }
    publish();
    informLib();
    informUI();
}
