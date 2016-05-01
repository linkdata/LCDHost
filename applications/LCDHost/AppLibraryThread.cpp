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
#include <QApplication>

#include "AppLibraryThread.h"
#include "AppLibrary.h"
#include "LibLibrary.h"

#include <signal.h>

void lh_thread_catch_sig(int s)
{
    if (!QThread::currentThread()) {
        qWarning() << "Exception " << s << "in unknown OS thread";
        raise(s);
        return;
    }
    if(QThread::currentThread() == qApp->thread()) {
        qWarning() << "Exception in main thread";
        raise(s);
        return;
    }
    if(AppLibraryThread* thread = qobject_cast<AppLibraryThread *>(QThread::currentThread()))
    {
        signal( s, lh_thread_catch_sig );
        longjmp( thread->context_, s );
        return;
    }
    qWarning() << "Exception" << s << "in unknown QThread"
               << QThread::currentThreadId()
               << QThread::currentThread()->objectName();
    for(;;) QThread::sleep(1000);
}

AppLibraryThread::AppLibraryThread( AppLibrary *al ) : QThread(0)
{
    app_library_ = al;
    lib_library_ = NULL;
    setObjectName( al->objectName() ); // not really needed, but good when debugging
    crashed_ = false;
    return;
}

AppLibraryThread::~AppLibraryThread()
{
    app_library_ = NULL;
    setLibLibrary(NULL);
}

void AppLibraryThread::setCrashed()
{
    // note: can't delete lib_library_ here, since it's not created in this thread!
    crashed_ = true;
    setLibLibrary(NULL);
}

void AppLibraryThread::setLibLibrary( LibLibrary *p )
{
    mutex_.lock();
    lib_library_ = p;
    mutex_.unlock();
    return;
}

void AppLibraryThread::run()
{
    int sig;
    QString libname;
    LibLibrary *actual_lib = 0;

#ifdef SIGSEGV
    signal( SIGSEGV, lh_thread_catch_sig );
#endif
#ifdef SIGILL
    signal( SIGILL, lh_thread_catch_sig );
#endif
#ifdef SIGFPE
    signal( SIGFPE, lh_thread_catch_sig );
#endif
#ifdef SIGPIPE
    signal( SIGPIPE, lh_thread_catch_sig );
#endif
#ifdef SIGBUS
    signal( SIGBUS, lh_thread_catch_sig );
#endif
    sig = setjmp( context_ );

    if( sig == 0 )
    {
        if( lh_log_load )
            qDebug() << QString("<span style=\"background-color: #f0f0f0;\">Thread</span> <tt>%1</tt> AppLibraryThread::run()")
                    .arg(objectName());

        // load it here so global objects in the DLL
        // gets created under the correct thread
        if( app_library_->lib()->load() )
            actual_lib = new LibLibrary( app_library_->lib(), app_library_->id() );

        if( actual_lib )
        {
            libname = actual_lib->objectName();
            QString errmsg = actual_lib->init();
            app_library_->setError(errmsg);
            if( errmsg.isEmpty() )
            {
                setLibLibrary( actual_lib );
                app_library_->clearTimeout();
                app_library_->setState( AppLibrary::Loaded );
                actual_lib->loadClassList();
                actual_lib->loadComplete();
                qDebug() << libname << "loaded with" << actual_lib->classCount() << "classes";
                exec();
                setLibLibrary( NULL );
                app_library_->setState( AppLibrary::Unloaded );
            }
            else
            {
                qWarning() << libname << "returned error message" << errmsg;
                app_library_->setState( AppLibrary::Unloaded );
            }
            actual_lib->term();
            delete actual_lib;
            actual_lib = NULL;
            qDebug() << libname << "unloading";
            app_library_->unload();
        }
        else
            app_library_->setState( AppLibrary::Invalid );
    }
    else
    {
        setLibLibrary( NULL );
        delete actual_lib;
        app_library_->addCrash();
        app_library_->setState( AppLibrary::Failed );
        app_library_->unload();
        switch( sig )
        {
#ifdef SIGSEGV
        case SIGSEGV:
            qWarning() << objectName() << "caused a segmentation violation";
            break;
#endif
#ifdef SIGILL
        case SIGILL:
            qWarning() << objectName() << "executed an illegal instruction";
            break;
#endif
#ifdef SIGFPE
        case SIGFPE:
            qWarning() << objectName() << "tried to divide by zero";
            break;
#endif
#ifdef SIGPIPE
        case SIGPIPE:
            qWarning() << objectName() << "received SIGPIPE";
            break;
#endif
#ifdef SIGBUS
        case SIGBUS:
            qWarning() << objectName() << "received SIGBUS";
            break;
#endif
        default:
            qWarning() << objectName() << "received signal" << sig;
            break;
        }
    }

    return;
}

