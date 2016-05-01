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

#ifndef APPLIBRARYTHREAD_H
#define APPLIBRARYTHREAD_H

#include <setjmp.h>
#include <QThread>
#include <QTime>
#include <QFileInfo>
#include <QMutex>
#include <QEvent>
#include <QLibrary>
#include <QCoreApplication>

#include "AppId.h"
class AppLibrary;
class LibLibrary;

// Thread that insulates the application from the
// thread as best it can.

class AppLibraryThread : public QThread
{
    Q_OBJECT

    QMutex mutex_; // protects lib_library_ & app_library_
    bool crashed_; // true if a crash has been detected
    jmp_buf context_;
    AppLibrary *app_library_;
    LibLibrary *lib_library_; // public LibLibrary pointer

    void run();

    friend void lh_thread_catch_sig(int s);

public:
    AppLibraryThread( AppLibrary *al );
    ~AppLibraryThread();

    bool crashed() const { return crashed_; }
    void setCrashed();

    AppLibrary *appLibrary() const { return app_library_; }
    LibLibrary *libLibrary() const { return lib_library_; }
    void setLibLibrary( LibLibrary * );
};

#endif // APPLIBRARYTHREAD_H
