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

#ifndef APPLIBRARY_H
#define APPLIBRARY_H

#include <QObject>
#include <QFileInfo>
#include <QByteArray>
#include <QImage>
#include <QLibrary>
#include <QMutex>

#include "LCDHost.h"
#include "AppId.h"
#include "AppObject.h"
#include "lh_plugin.h"
#include "PluginInfo.h"
#include "AppPluginVersion.h"

#if LH_USE_THREADS
# include <QMutex>
# include <QMutexLocker>
# include "AppLibraryThread.h"
#else
# include "LibLibrary.h"
#endif

class AppObject;
class AppClass;
class AppState;

class AppLibrary : public AppObject
{
    Q_OBJECT

    PluginInfo pi_;
    QString error_;
    QByteArray logo_data_;
    QImage logo_;
    QLibrary *lib_;

#if LH_USE_THREADS
    AppLibraryThread *thread_;
    int timeout_;           //< Timeout counter
    unsigned crashes_;      //< Crash count
    QMutex mutex_; // used when accessing the children() list
#else
    LibLibrary* liblibrary_;
#endif

    // QList<AppClass*> classlist_;
    bool seen_devices_;     //< true if we've seen a device from this library
    bool seen_classes_;     //< true if we've seen classes from this library

public:
    enum State {
        Invalid,        // not loaded, not a library
        Unloaded,       // not loaded
        Failed,         // not loaded, crashed
        Loaded,         // loaded and running
        Orphanage       // the orphanage
    };

    static QString simplenameFromFileInfo( QFileInfo fi );
    static AppLibrary *fromName(const QString &name);
    static AppLibrary *fromName( QVariant v )
    {
        return fromName( v.toString() );
    }
    static AppLibrary *fromId( AppId id )
    {
        return qobject_cast<AppLibrary *>(id.appObject());
    }
    static AppLibrary *orphanage();
    static AppClass *getClass( const QString &classId );
    // static QList<AppLibrary*> list();

    explicit AppLibrary(QObject *parent);
    explicit AppLibrary(const PluginInfo& pi, AppObject *parent);
    ~AppLibrary();

    void init();
    void term();

    QString path() const;

    PluginInfo& pi() { return pi_; }
    QString name() const { return pi_.id(); }
    QString description() const { return pi_.shortdesc(); }
    QString author() const { return pi_.author(); }
    QString comments() const { return pi_.longdesc(); }
    QString homepage() const { return pi_.homepageurl(); }
    QImage logo() const { return QImage(); /*pi_.logourl();*/ }

    int revision() const { return pi_.revision(); }
    QString version() const { return pi_.version(); }
    QUrl versionURL() const { return AppPluginVersion::makeUrl( pi_.versionurl(), objectName(), pi_.revision() ); }

    QString error() const { return error_; }

    void setError(const QString& s );

    void loadSettings();
    void saveSettings();

    const QFileInfo& fileInfo() const { return pi_.fileInfo(); }

    QString apiVersion() const { return QString("%1.%2").arg(pi_.api_major()).arg(pi_.api_minor()); }

    void setState( State s );
    State state() const { return state_; }

    bool isDriver() const { return !seen_classes_ || seen_devices_; }
    bool isPlugin() const { return seen_classes_; }

    bool isStarted() const { return state_ == Loaded; }

    bool isEnabled() const { return pi_.isEnabled(); }
    bool isUsable() const { return pi_.isUsable(); }
    void setEnabled( bool b ) { pi_.setEnabled(b); }

    // AppLibraryThread *libThreadCreate() { return new AppLibraryThread( this, fileinfo_ ); }
    QLibrary *lib() { return lib_; }

#if LH_USE_THREADS
    AppLibraryThread *libThread() const { return thread_; }
    void sendQuit() { if( thread_ && thread_->isRunning() ) thread_->quit(); }
    bool running() const { return thread_ ? (thread_->libLibrary() ? true : false) : false; }
    void clearTimeout() { timeout_ = 0; }
    int timeout() { return ++ timeout_; }
    unsigned crashes() const { return crashes_; }
    void clearCrashes() { crashes_ = 0; }
    void addCrash() { crashes_++; }
#else
    void sendQuit() {}
    bool running() const { return lib_->isLoaded(); }
    void clearTimeout() {}
    int timeout() { return 0; }
    unsigned crashes() const { return 0; }
    void clearCrashes() {}
    void addCrash() {}
#endif
    void start();
    void abort();
    void stop();
    void unload(); // unloading of the shared library / DLL

    void bindClass( AppClass *app_class );

    bool event( QEvent * );

    AppObject *obj( AppId id );
    AppObject *obj( QString name );
    QList<AppClass *> classes() const;

signals:
    void stateChanged( AppId id );
    void classAdded( QStringList path, QString id );
    void classRemoved( QStringList path, QString id );

public slots:
    void layoutChanged( QString dir, QString name );
    void requestWebUpdate();
#if LH_USE_THREADS
    void threadFinished();
#endif

private:
    State state_;           //< Current state
};

#endif // APPLIBRARY_H
