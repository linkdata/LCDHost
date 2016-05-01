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


#include <QDebug>
#include <QCoreApplication>
#include <QGridLayout>
#include <QToolButton>
#include <QLabel>
#include <QEvent>
#include <QDir>

#include "LCDHost.h"
#include "AppObject.h"
#include "AppInstance.h"
#include "AppSetupItem.h"
#include "AppSetupItemHelp.h"
#include "AppScriptEditor.h"
#include "EventObjectCreated.h"
#include "EventSetupItemEnum.h"
#include "EventSetupItem.h"
#include "EventSetupItemRefresh.h"
#include "EventRawInput.h"
#include "EventSetupComplete.h"
#include "EventLayoutLoad.h"
#include "AppRawInput.h"
#include "AppSourceLinkButton.h"
#include "MainWindow.h"
#include "EventObjectDestroy.h"

AppObject::AppObject(QObject *parent)
    : QObject(parent)
    , id_(this)
    , eventcount_(0)
    , appsetup_done_(false)
    , libobject_created_(false)
    , ready_(false)
{
}

AppObject::~AppObject()
{
#ifndef QT_NO_DEBUG
    if( id().hasAppObject() )
        qWarning() << id() << "has not been term()-inated";
#endif
    return;
}

void AppObject::init()
{
    Q_ASSERT( !id_.hasAppObject() );
    appsetup_done_ = false;
    ready_ = false;
    id_.setAppObject(this);
    return;
}

void AppObject::term()
{
    appsetup_done_ = false;
    ready_ = false;
    if( id().hasLibObject() )
        id().postLibEvent( new EventObjectDestroy() );
    if( id().hasAppObject() ) id().setAppObject(0);
    else qWarning() << "AppObject::term()" << metaObject()->className() << objectName() << "not initialized";
    return;
}

void AppObject::setObjectName( const QString name )
{
    QString copy = name;
    QObject::setObjectName( copy.replace('/','-') );
    return;
}

QString AppObject::path() const
{
    if( AppObject * p = qobject_cast<AppObject *>(parent()) )
        return p->path().append('/').append( objectName() );
    return QString('/');
}

void AppObject::setId( AppId id )
{
    id_ = id;
    return;
}

// At this point, the LibObject has sent all of the setup items it has,
// while we've been blocking updates, but marking which ones are still
// valid to use among those loaded off XML. Now we iterate over the
// valid items, and refresh the instance data with what we got.

// Called by loader when all of the data has been set into our
// ASI's. Mark 'appsetup_done_' as true.
void AppObject::setupComplete()
{
    // will fail if LibObject not yet created,
    // but then it'll fire on receiving EventObjectCreated instead
    ready_ = false;
    appsetup_done_ = true;
    if( libobject_created_ )
        id().postLibEvent( new EventSetupItemEnum() );
}

QWidget *AppObject::setupBuildUI(QWidget *parent)
{
    QWidget *setup_widget = new QWidget(parent);
    int row = -1;
    bool got_stretcher = false;

    // Build setup widget
    // Column layout:
    // [link] [help] [title] [value]
    QGridLayout *grid = new QGridLayout(setup_widget);
    // grid->setSizeConstraint(QLayout::SetNoConstraint);
    grid->setColumnStretch(0,0);
    grid->setColumnStretch(1,0);
    grid->setColumnStretch(2,0);
    grid->setColumnStretch(3,1);

    foreach( AppSetupItem *asi, setup_items_ )
    {
        if( asi && (!asi->isValid() || !asi->isVerified()))
        {
            setup_items_.removeAll( asi );
            continue;
        }

        ++ row;

        asi->buildUI( grid, row );

        if( asi->canStretch() )
        {
            grid->setRowStretch( row, 1 );
            got_stretcher = true;
        }
    }

    if( !got_stretcher )
    {
        ++row;
        grid->addWidget( new QWidget(setup_widget), row, 0, -1, -1 );
        grid->setRowStretch( row, 1 );
    }

    return setup_widget;
}

void AppObject::setupFocusUI()
{
    foreach( AppSetupItem *asi, setup_items_ )
        if( asi && asi->setFocus() ) return;
}

void AppObject::input( EventRawInput *evt )
{
    foreach( AppSetupItem *asi, setup_items_ )
    {
        if( asi && (asi->type() == lh_type_string_inputstate || asi->type() == lh_type_string_inputvalue) )
        {
            if( !asi->extraInt() || asi->extraInt() == evt->flags )
            {
                if( asi->extraStr() == evt->control )
                {
                    EventRawInput *copy = new EventRawInput( *evt );
                    copy->receiver = this->id();
                    id_.postLibEvent( copy, Qt::HighEventPriority+1 );
                }
            }
        }
    }
}

bool AppObject::event( QEvent *event )
{
    if( event->type() < QEvent::User || event->type() > QEvent::MaxUser )
    {
        QObject::event(event);
        return true;
    }

    if( id().isEmpty() )
    {
        qDebug() << "AppObject::event() : dead event" << EventBase::name(event->type()) << "for" << objectName();
        return true;
    }

    if( event->type() == EventSetupItemRefresh::type() )
    {
        // Used to send setup refreshes from LibInstance to AppInstance and vice versa
        // Refresh from the LibInstance. Only accept data if we're done
        // initializing the LibInstance, but always let it update
        // parameters (list info in particular)
        EventSetupItemRefresh *e = static_cast<EventSetupItemRefresh*>(event);
        foreach( AppSetupItem *asi, setup_items_ )
        {
            if( asi && asi->name() == e->lsi.name() )
            {
                if( ready_ ) asi->fromLib( &e->lsi );
                else asi->verified( &e->lsi );
                break;
            }
        }
        return true;
    }

    if( event->type() == EventObjectCreated::type() )
    {
        // signal LibObject to send EventSetupItem messages
        // for all it's setup items it has from the plugin.
        ready_ = false;
        libobject_created_ = true;
        if( appsetup_done_ )
            id().postLibEvent( new EventSetupItemEnum() );
        return true;
    }

    if( event->type() == EventSetupItem::type() )
    {
        // Sent by LibObject to AppObject to inform about a setup item from the plugin
        EventSetupItem *e = static_cast<EventSetupItem*>(event);
        AppSetupItem *asi = 0;
        int index = -1;

        ready_ = false;

        if( lh_log_load )
            qDebug() << QString("<span style=\"background-color: #f0f0f0;\">Loading</span> <tt>%1</tt> AppObject::eventSetupItem() for <tt>%2</tt>")
                    .arg(objectName())
                    .arg(e->lsi.name());

        foreach( asi, setup_items_ )
        {
            ++ index;
            if (!asi) {
              qWarning() << objectName() << "has NULL setup item at index" << index;
              continue;
            }
            if( asi->name() == e->lsi.name() )
            {
                setup_items_.move( index, setup_items_.size()-1 );
                asi->verified( &e->lsi );
                return true;
            }
        }

        asi = AppSetupItem::fromLibSetupItem( this, &e->lsi );
        if (asi)
          setup_items_.append( asi );
        else
          qWarning() << objectName() << "returned invalid setup item" << e->lsi.name();

        return true;
    }

    if( event->type() == EventSetupItemEnum::type() )
    {
        // LibObject has sent all it's setup items - send back stored values

        ready_ = false;

        if( lh_log_load )
            qDebug() << QString("<span style=\"background-color: #f0f0f0;\">Loading</span> <tt>%1</tt> Setup item enumeration complete")
                    .arg(objectName());

        foreach( AppSetupItem *asi, setup_items_ )
        {
            if( asi && asi->isValid() )
            {
                if( asi->isVerified() )
                {
                    EventSetupItemRefresh * initial = new EventSetupItemRefresh( asi );
                    if( asi->flags() & LH_FLAG_NOSAVE_DATA )
                        initial->lsi.clearValue();
                    if( asi->flags() & LH_FLAG_NOSAVE_LINK )
                    {
                        initial->lsi.clearSubscribe();
                        initial->lsi.clearPublish();
                    }
                    id().postLibEvent( initial );
                    asi->pullSource();
                }
                else
                    qDebug() << objectName() << "has unused setup item" << asi->name();
            }
            else
                qDebug("%s \"%s\": setup item \"%s\" (%s) is invalid",
                       metaObject()->className(),
                       qPrintable(objectName()),
                       qPrintable(asi->name()),
                       AppSetupItem::typeName(asi->type())
                       );
        }

        id().postLibEvent( new EventSetupComplete(id()) );

        if( mainWindow )
            QCoreApplication::postEvent( reinterpret_cast<QObject*>(mainWindow), new EventSetupComplete(id()) );

        return true;
    }

    if( event->type() == EventSetupComplete::type() )
    {
        // Sent from LibObject to confirm receipt of the same message
        // meaning all confirm load values have been recieved by it
        // and it's done the inital polling() and notify() calls
        if( lh_log_load )
            qDebug() << QString("<span style=\"background-color: #f0f0f0;\">Loading</span> <tt>%1</tt> AppObject::eventSetupComplete()")
                    .arg(objectName());
        foreach( AppSetupItem *asi, setup_items_ )
        {
            if(asi && asi->isValid() && asi->isVerified())
            {
                asi->informUI();
                asi->publish();
            }
        }
        ready();
        ready_ = true;
        return true;
    }

    if( event->type() == EventLayoutLoad::type() )
    {
        // Sent by a library to request layout change
        if( mainWindow )
        {
            EventLayoutLoad *e = static_cast<EventLayoutLoad*>(event);
            if (AppState* as = AppState::instance()) {
              QFileInfo fi(QDir(as->dir_data()), e->name);
              if( fi.exists() )
              {
                  mainWindow->layoutPath = fi.path();
                  mainWindow->layoutName = fi.fileName();
                  mainWindow->loadLayout();
              }
              else
              {
                  qDebug() << objectName() << "tried to load nonexistant layout" << e->name;
              }
            }
        }
        return true;
    }

    return false;
}
