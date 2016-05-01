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

#include <QtGlobal>
#include <QtDebug>
#include <QDir>
#include <QSettings>
#include <QEvent>
#include <QBuffer>

#include "LCDHost.h"
#include "MainWindow.h"
#include "AppState.h"
#include "AppLibrary.h"
#include "LibLibrary.h"
#include "LibObject.h"
#include "AppLibraryThread.h"
#include "AppDevice.h"
#include "AppInstance.h"
#include "AppClass.h"
#include "AppRawInput.h"

#include "EventDeviceCreate.h"
#include "EventDeviceDestroy.h"
#include "EventLibraryLoaded.h"
#include "EventLayoutChanged.h"
#include "EventClassLoaded.h"
#include "EventClassRemoved.h"
#include "EventCreateAppDevice.h"
#include "EventDestroyAppDevice.h"
#include "EventCreateInstance.h"
#include "EventLayoutLoad.h"

QString AppLibrary::simplenameFromFileInfo( QFileInfo fi )
{
    QString simplename = fi.baseName();
    QDir plugindir( AppState::instance()->dir_plugins() );
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    if( simplename.startsWith("lib") ) simplename.remove(0,3);
#endif
    Q_ASSERT(simplename == simplename.trimmed());
    simplename = simplename.trimmed();
    if( fi.dir() == plugindir ) return simplename;
    QFileInfo fi2( plugindir.relativeFilePath( fi.filePath() ) );
    return fi2.path() + "/" + simplename;
}

AppObject *AppLibrary::obj( AppId id )
{
#if LH_USE_THREADS
    QMutexLocker locker(&mutex_);
#endif
    foreach( QObject *o, children() )
    {
        AppObject *app_obj = static_cast<AppObject *>(o);
        if( app_obj->id() == id )
            return app_obj;
    }
    return 0;
}

AppObject *AppLibrary::obj( QString name )
{
#if LH_USE_THREADS
    QMutexLocker locker(&mutex_);
#endif
    foreach( QObject *o, children() )
    {
        AppObject *app_obj = static_cast<AppObject *>(o);
        if( app_obj->objectName() == name )
            return app_obj;
    }
    return 0;
}

QList<AppClass *> AppLibrary::classes() const
{
    return findChildren<AppClass *>();
}

AppLibrary *AppLibrary::fromName(const QString &name)
{
    if(AppLibrary *app_lib = mainWindow->pluginParent()->findChild<AppLibrary *>(name, Qt::FindDirectChildrenOnly))
        return app_lib;
    if(orphanage()->objectName().compare(name, Qt::CaseInsensitive))
        return orphanage();
    foreach(AppLibrary *app_lib, mainWindow->pluginParent()->findChildren<AppLibrary *>(QString(), Qt::FindDirectChildrenOnly))
        if(app_lib->objectName().compare(name, Qt::CaseInsensitive) == 0)
            return app_lib;
    return NULL;
}

AppClass *AppLibrary::getClass(const QString &classId)
{
    return mainWindow->pluginParent()->findChild<AppClass *>(classId);
}

AppLibrary *AppLibrary::orphanage()
{
    static AppLibrary* orphanage_ = new AppLibrary(mainWindow);
    return orphanage_;
}

AppLibrary::AppLibrary(QObject *parent)
    : AppObject(parent)
    , lib_(0)
#if LH_USE_THREADS
    , thread_(0)
    , timeout_(0)
    , crashes_(0)
#else
    , liblibrary_(0)
#endif
    , seen_devices_(false)
    , seen_classes_(false)
    , state_(Orphanage)
{
    Q_ASSERT(mainWindow);
    Q_ASSERT(mainWindow == parent);
    setObjectName(QLatin1String("orphanage"));
}

AppLibrary::AppLibrary(const PluginInfo& pi, AppObject *parent)
    : AppObject(parent)
    , lib_(0)
#if LH_USE_THREADS
    , thread_(0)
    , timeout_(0)
    , crashes_(0)
#else
    , liblibrary_(0)
#endif
    , seen_devices_(false)
    , seen_classes_(false)
    , state_(Unloaded)
{
    Q_ASSERT(mainWindow);
    Q_ASSERT(mainWindow->pluginParent() == parent);

    pi_ = pi;
    setObjectName(simplenameFromFileInfo(pi_.fileInfo()));
    loadSettings();
    return;
}

AppLibrary::~AppLibrary()
{
    abort();
    saveSettings();
    return;
}

void AppLibrary::init()
{
    AppObject::init();
    connect( this, SIGNAL(stateChanged(AppId)), mainWindow, SLOT(stateChanged(AppId)) );
    connect( this, SIGNAL(classAdded(QStringList,QString)), mainWindow->classTree(), SLOT(insertClass(QStringList,QString)) );
    connect( this, SIGNAL(classRemoved(QStringList,QString)), mainWindow->classTree(), SLOT(removeClass(QStringList,QString)) );
    connect( mainWindow, SIGNAL(layoutChanged(QString,QString)), this, SLOT(layoutChanged(QString,QString)) );
    setupComplete();
    return;
}

void AppLibrary::term()
{
    if( id().hasAppObject() )
    {
        // Terminate devices (instance get terminated by QGraphicsScene or MainWindow)
        foreach( QObject *obj, children() )
        {
            AppDevice *app_obj = qobject_cast<AppDevice*>(obj);
            if( app_obj ) app_obj->term();
        }
        AppObject::term();
    }
    else
        qWarning() << "AppLibrary::term()" << metaObject()->className() << objectName() << "not initialized";
}

QString AppLibrary::path() const
{
    return QString("/plugin/").append( objectName() );
}

void AppLibrary::loadSettings()
{
    if( !fileInfo().completeBaseName().isEmpty() )
    {
        QSettings settings;
        QString setupitems;
        settings.beginGroup( "plugins" );
        settings.beginGroup( fileInfo().completeBaseName() );
        logo_.loadFromData( settings.value( "logo", QByteArray() ).toByteArray() );
#if LH_USE_THREADS
        crashes_ = settings.value( "crashes", 0 ).toUInt();
#endif
        settings.beginGroup("setupitems");
        foreach(const QString& key, settings.childGroups())
        {
            AppSetupItem *asi = AppSetupItem::fromSettings(this, settings, key);
            if(asi) setup_items_.append( asi );
        }
        settings.endGroup();
        settings.endGroup();
        settings.endGroup();
    }
    return;
}

void AppLibrary::saveSettings()
{
    if( state() != Invalid && state() != Orphanage &&
        !fileInfo().completeBaseName().isEmpty() )
    {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        QSettings settings;

        settings.beginGroup( "plugins" );
        settings.beginGroup( fileInfo().completeBaseName() );
        if( !logo_.isNull() )
        {
            buffer.open(QIODevice::WriteOnly);
            logo_.save( &buffer, "PNG" );
            settings.setValue( "logo", bytes );
        }

#if LH_USE_THREADS
        settings.setValue( "crashes", crashes_ );
#endif

        if( !setup_items_.isEmpty() )
        {
            settings.beginGroup("setupitems");
            foreach( AppSetupItem *asi, setup_items_ )
                if (asi) asi->save( settings );
            settings.endGroup();
        }
        else
            settings.remove("setupitems");

        settings.endGroup();
        settings.endGroup();
    }

    return;
}

void AppLibrary::setState( State s )
{
    if( s == state_ ) return;
    state_ = s;

    if( state() == AppLibrary::Failed )
    {
        foreach( QObject *o, children() )
        {
            AppDevice *app_dev = qobject_cast<AppDevice *>(o);
            if( app_dev )
            {
                app_dev->term();
                delete app_dev;
            }
        }
    }

    emit stateChanged( id() );
    return;
}

void AppLibrary::setError(const QString& s) {
    if (error_ != s) {
        error_ = s;
        emit stateChanged( id() );
    }
}

void AppLibrary::unload()
{
#ifdef Q_OS_MAC
//    if( this->thread() == QThread::currentThread() && lib_->isLoaded() )
#endif
        lib_->unload();
}

void AppLibrary::start()
{
#if LH_USE_THREADS
    Q_ASSERT( id().hasAppObject() );

    clearTimeout();

    if( thread_ == NULL )
    {
        Q_ASSERT( lib_ == NULL );
        if( lib_ == NULL )
            lib_ = new QLibrary( fileInfo().filePath() );
        thread_ = new AppLibraryThread( this );
        connect( thread_, SIGNAL(finished()), this, SLOT(threadFinished()) );
        thread_->start();
    }
#else
    Q_ASSERT(!lib_);
    Q_ASSERT(!liblibrary_);

    if( lh_log_load )
        qDebug() << QString("<tt>%1</tt> AppLibrary::start()").arg(objectName());

    lib_ = new QLibrary(fileInfo().filePath());
    lib_->setObjectName(objectName());
    if(lib_->load()) {
        liblibrary_ = new LibLibrary(lib_, id());
        QString errmsg = liblibrary_->init();
        if(errmsg.isEmpty()) {
            setState(Loaded);
            liblibrary_->loadClassList();
            liblibrary_->loadComplete();
            qDebug() << liblibrary_->objectName() << "loaded with" << liblibrary_->classCount() << "classes";
            return;
        }
        setState(Unloaded);
        qWarning() << liblibrary_->objectName() << "returned error message" << errmsg;
        setError(errmsg);
        liblibrary_->term();
        delete liblibrary_;
        liblibrary_ = 0;
        lib_->unload();
    } else {
        setState(Invalid);
        qCritical() << lib_->objectName() << "failed to load";
    }
    delete lib_;
    lib_ = 0;

    /*
    setLibLibrary( NULL );
    app_library_->setState( AppLibrary::Unloaded );
    liblibrary_->term();
    delete liblibrary_;
    liblibrary_ = 0;
    qDebug() << lib_->objectName() << "unloading";
    app_library_->unload();
    */
#endif
    return;
}

void AppLibrary::abort()
{
#if LH_USE_THREADS
    if( thread_ )
    {
        thread_->quit();
        clearTimeout();
        for( int waited = 0; thread_ && thread_->isRunning(); ++ waited )
        {
            QCoreApplication::processEvents();
            if( thread_ && ! thread_->wait(100) && waited > 50 )
            {
                waited = 40;
                qCritical( "'%s' is not responding",  qPrintable(objectName()) );
                setState(AppLibrary::Failed);
            }
        }
    }
#else
    stop();
#endif
    return;
}

void AppLibrary::stop()
{
#if LH_USE_THREADS
    if( thread_ )
    {
        clearTimeout();
        thread_->quit();
    }
#else
    if (liblibrary_) {
        liblibrary_->term();
        delete liblibrary_;
        liblibrary_ = 0;
    }
    if (lib_) {
        if (lib_->isLoaded()) {
            qDebug() << lib_->objectName() << "unloading";
            if (!lib_->unload()) {
                qDebug() << lib_->objectName() << "failed to unload";
                return;
            }
        }
        Q_ASSERT(!lib_->isLoaded());
        setState(Unloaded);
        delete lib_;
        lib_ = 0;
    }
#endif
    return;
}

#if LH_USE_THREADS
void AppLibrary::threadFinished()
{
    if( thread_ )
    {
        delete thread_;
        thread_ = NULL;
    }

    if( lib_ )
    {
        if( lib_->isLoaded() ) lib_->unload();
        delete lib_;
        lib_ = NULL;
        setState( Unloaded );
    }
}
#endif

void AppLibrary::layoutChanged( QString dir, QString name )
{
    id().postLibEvent( new EventLayoutChanged(dir,name) );
    return;
}

void AppLibrary::bindClass( AppClass *app_class )
{
    foreach( AppObject *app_obj, AppId::appObjects() )
    {
        AppInstance *app_inst = qobject_cast<AppInstance*>( app_obj );
        if( app_inst && app_inst->classId() == app_class->id() )
        {
            if( !app_inst->id().hasLibObject() )
            {
                app_inst->setParent( this );
                app_inst->setClassSize( app_class->size() );
                id().postLibEvent( new EventCreateInstance( app_class->id(), app_inst->id(), app_inst->objectName() ) );
            }
        }
    }

    return;
}


void AppLibrary::requestWebUpdate()
{
    if( mainWindow ) mainWindow->webUpdatePlugin( objectName() );
}

bool AppLibrary::event(QEvent *event)
{
    if( AppObject::event(event) ) return true;
    Q_ASSERT( event->type() >= QEvent::User );
    Q_ASSERT( event->type() <= QEvent::MaxUser );
    Q_ASSERT( id().isValid() );

    if( event->type() == EventLibraryLoaded::type() )
    {
#if LH_USE_THREADS
        if( thread_ )
        {
            if( thread_->crashed() )
            {
                Q_ASSERT( ! thread_->isRunning() );
                if( state() != Invalid ) setState( Failed );
            }
            else if( thread_->isRunning() )
            {
                setState( Loaded );
            }
        }
#endif
        return true;
    }

    if( event->type() == EventClassLoaded::type() )
    {
        // create a corresponding AppClass
        EventClassLoaded *e = static_cast<EventClassLoaded*>(event);
        AppClass *app_class = new AppClass( this, e->id, e->path, e->name, e->size );
        seen_classes_ = true;
        if( app_class )
        {
            if( lh_log_load )
                qDebug() << QString("<span style=\"background-color: #f0f0f0;\">Classes</span> <tt>%1</tt> Class %2 recieved (%3x%4)")
                            .arg(objectName()).arg( app_class->id() )
                            .arg(app_class->size().width())
                            .arg(app_class->size().height())
                            ;
            // classlist_.append( app_class );
            bindClass( app_class );
            emit classAdded( app_class->path(), app_class->id() );
        }
        return true;
    }

    if( event->type() == EventClassRemoved::type() )
    {
        EventClassRemoved *e = static_cast<EventClassRemoved*>(event);
        foreach(AppClass *app_class, findChildren<AppClass *>(e->id, Qt::FindDirectChildrenOnly))
        {
            emit classRemoved( app_class->path(), app_class->id() );
            delete app_class;
        }
        return true;
    }

    if( event->type() == EventCreateAppDevice::type() )
    {
        EventCreateAppDevice *e = static_cast<EventCreateAppDevice*>(event);
        AppDevice *app_dev = NULL;
        seen_devices_ = true;
        app_dev = new AppDevice(this,e->devid,e->name,e->size,e->depth,e->autoselect,e->logo);
        app_dev->init();
        id().postLibEvent( new EventDeviceCreate(app_dev->id(), app_dev->devId()) );
        app_dev->setupComplete();
        QCoreApplication::postEvent( mainWindow, new EventDeviceCreate( app_dev->id(), app_dev->devId() ));
        return true;
    }

    if( event->type() == EventDestroyAppDevice::type() )
    {
        EventDestroyAppDevice *e = static_cast<EventDestroyAppDevice*>(event);
        foreach( QObject *obj, children() )
        {
            AppDevice *app_dev = qobject_cast<AppDevice*>( obj );
            if( app_dev && app_dev->devId() == e->devid )
            {
                QCoreApplication::postEvent( mainWindow, new EventDeviceDestroy( app_dev->devId() ));
                app_dev->term();
                delete app_dev;
            }
        }
        return true;
    }

    if( event->type() == EventRawInput::type() )
    {
        // Sent by devices when their buttons change
        EventRawInput *e = static_cast<EventRawInput*>(event);
        if (AppRawInput* ri = AppRawInput::instance())
          ri->broadcast(e);
    }

    qWarning() << "AppLibrary::event() unhandled user event" << EventBase::name(event->type()) << "for" << objectName();
    return false;
}
