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

#ifndef RAWINPUTENGINE_WIN_H
#define RAWINPUTENGINE_WIN_H

#include <QtGlobal>
#include <QDateTime>

#ifndef WINVER
# define WINVER 0x0501
#endif

#include <windows.h>
#include "RawInputEngine.h"

class AppRawInput;
class RawInputDevice_win;

class RawInputEngine_win : public RawInputEngine
{
public:
    RawInputEngine_win( AppRawInput *p );
    ~RawInputEngine_win();

    void clear();
    void scan();

    bool timetodie() const { return timetodie_; }
    const char* HidP_error( int status );

    void ensureRegistrations();

    static void setWorkerWindow( HWND hWnd ) { worker_window_ = hWnd; }
    static HWND workerWindow() { return worker_window_; }
    static RawInputDevice_win* getDevice( HANDLE h );

private:
    static HWND worker_window_;
    UINT input_buffer_size_;
    DWORD worker_thread_id;
    HANDLE worker_thread;
    bool timetodie_;
};

#endif // RAWINPUTENGINE_WIN_H
