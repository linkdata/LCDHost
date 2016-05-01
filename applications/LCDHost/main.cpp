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
#include <QApplication>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QSettings>
#include <QScopedPointer>
#include <QSystemTrayIcon>

#ifdef Q_OS_WIN
# ifndef _WIN32_WINNT
#  define _WIN32_WINNT 0x0501 // 0x0501 WinXP, 0x0601 WinVista
# endif
# include <windows.h>
#endif

#include "LCDHost.h"
#include "../lh_logger/LH_Logger.h"
#include "AppState.h"
#include "AppId.h"
#include "MainWindow.h"
#include "AppLibraryThread.h"
#include "AppSetupItem.h"
#include "AppInstanceTree.h"
#include "RemoteWindow.h"
#include "EventAppStartup.h"

#if LH_USE_HID
#include "LH_HidThread.h"
#endif

// Globals
MainWindow *mainWindow = NULL;
LH_RenderMethod lh_rendermethod = LH_RenderMethod_Pixmap;

#ifndef layoutModified
void layoutModified()
{
    if( mainWindow ) mainWindow->setWindowModified( true );
}
#endif

int msecDiff( const QDateTime& from, const QDateTime& to )
{
    int diff;
    if( !to.isValid() ) return INT_MAX;
    diff = (1000 * 60 * 60 * 24) * from.date().daysTo( to.date() );
    diff += from.time().msecsTo( to.time() );
    return diff;
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(LCDHostResources);

    QCoreApplication::setAttribute(Qt::AA_ImmediateWidgetCreation, true);
#ifdef Q_OS_MAC
    QCoreApplication::setAttribute(Qt::AA_NativeWindows, true);
#endif

    // QScopedPointer<QApplication> app(new QApplication(argc, argv));
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Link Data");
    QCoreApplication::setOrganizationDomain("linkdata.se");
    QCoreApplication::setApplicationName("LCDHost");
    QCoreApplication::setApplicationVersion(VERSION);
    int retv = 0;
    QString layoutfile;
    bool start_hidden = false;
    bool disable_systray = false;

#ifdef Q_OS_LINUX
    disable_systray = true;
#endif

    qRegisterMetaType<AppId>("AppId");
    qsrand( QTime::currentTime().msec() + QTime::currentTime().second() );

    LH_Logger *logger = new LH_Logger(QLatin1String("LCDHost"));
    // AppState *app_state = new AppState(qApp);
    // QDir().mkpath(app_state->dir_data());

    // QCoreApplication::setLibraryPaths(QStringList(app_state->dir_plugins()));
    // qRegisterMetaType<AppSetupItem>("AppSetupItem");
    // qRegisterMetaTypeStreamOperators<AppSetupItem>("AppSetupItem");

    // if the plugins/imageformats dir exists, use only that
    // QDir pluginsdir( AppState::instance()->dir_plugins() );
    // if( pluginsdir.exists("imageformats"))
    // QCoreApplication::setLibraryPaths( QStringList(AppState::instance()->dir_plugins()) );
    // else
    // QCoreApplication::addLibraryPath(AppState::instance()->dir_plugins());

    // parse arguments
    QStringList args = QCoreApplication::arguments();
    args.removeFirst();
    foreach(const QString& s, args)
    {
        if(!s.compare("--hidden")) {
            start_hidden = true;
            continue;
        }
        if(!s.compare("--disable-systray")) {
            disable_systray = true;
            continue;
        }
        if(!s.compare("--enable-systray")) {
            disable_systray = false;
            continue;
        }
        layoutfile = QDir::cleanPath(s);
    }

    qApp->setQuitOnLastWindowClosed(disable_systray);

    if (RemoteWindow* remote_window = new RemoteWindow()) {
        QScopedPointer<MainWindow> w(new MainWindow());

        logger->setParent(w.data());
        w->setSystrayDisabled(disable_systray);
        w->loadSettings();
        if (!layoutfile.isEmpty())
        {
            int last_slash = layoutfile.lastIndexOf(QChar('/'));
            if (last_slash != -1)
                w->layoutPath = layoutfile.left(last_slash);
            w->layoutName = layoutfile.mid(last_slash + 1);
        }

        if (start_hidden) {
          if (disable_systray || !QSystemTrayIcon::isSystemTrayAvailable())
            w->showMinimized();
        } else {
          w->show();
        }

        mainWindow = w.data();
        w->timer().start(0, w.data());
        retv = qApp->exec();
        mainWindow = 0;
        remote_window->close();
    }

    return retv;
}

QDataStream & operator>> ( QDataStream& stream, void *& p )
{
    quint64 n;
    stream >> n;
    p = (void*) (quintptr) n;
    return stream;
}

QDataStream & operator<< ( QDataStream& stream, const void * p )
{
    quint64 n;
    n = (quintptr) p;
    stream << n;
    return stream;
}

QDataStream & operator>> ( QDataStream& stream, AppInstance *& p )
{
    quint64 n;
    stream >> n;
    p = (AppInstance*) (quintptr) n;
    return stream;
}

QDataStream & operator<< ( QDataStream& stream, const AppInstance * p )
{
    quint64 n;
    n = (quintptr) p;
    stream << n;
    return stream;
}
