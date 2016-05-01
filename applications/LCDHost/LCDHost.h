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

#ifndef LCDHOST_H
#define LCDHOST_H

#include <QtGlobal>

typedef enum
{
    LH_RenderMethod_Image,
    LH_RenderMethod_Pixmap
} LH_RenderMethod;

// Globals
class RemoteWindow;
RemoteWindow *remoteLCDHostWindow(bool enabled = true);

class MainWindow;
extern MainWindow *mainWindow;
extern LH_RenderMethod lh_rendermethod;

#include <QtGlobal>
#include <QDataStream>
#include <QDateTime>

#define BUILD_YEAR ((((__DATE__ [7]-'0')*10+(__DATE__[8]-'0'))*10+(__DATE__ [9]-'0'))*10+(__DATE__ [10]-'0'))

#define BUILD_MONTH (__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6) \
              : __DATE__ [2] == 'b' ? 2 \
              : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
              : __DATE__ [2] == 'y' ? 5 \
              : __DATE__ [2] == 'l' ? 7 \
              : __DATE__ [2] == 'g' ? 8 \
              : __DATE__ [2] == 'p' ? 9 \
              : __DATE__ [2] == 't' ? 10 \
              : __DATE__ [2] == 'v' ? 11 : 12)

#define BUILD_DAY ((__DATE__ [4]==' ' ? 0 : __DATE__[4]-'0')*10+(__DATE__[5]-'0'))

#define LH_USE_HID 1

#if defined(Q_OS_WIN)
# define LH_USE_THREADS 1
# define LH_USE_SYSTRAY 1
#elif defined(Q_OS_MAC)
# define LH_USE_THREADS 1
# define LH_USE_SYSTRAY 1
#else
# define LH_USE_THREADS 1
# define LH_USE_SYSTRAY 1
#endif

// #define layoutModified() ( mainWindow ? mainWindow->setWindowModified( true ) : (void)0 )
extern void layoutModified();
int msecDiff( const QDateTime& from, const QDateTime& to );

void AppOSXInstall(); // from AppOSXInstall.cpp

// log stuff from AppDebugDialog
extern bool lh_log_load;

//

class AppInstance;

QDataStream & operator>> ( QDataStream& stream, void *& p );
QDataStream & operator<< ( QDataStream& stream, const void * p );
QDataStream & operator>> ( QDataStream& stream, AppInstance *& p );
QDataStream & operator<< ( QDataStream& stream, const AppInstance * p );

#endif // LCDHOST_H
