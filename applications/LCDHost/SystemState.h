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

#ifndef SYSTEMSTATE_H
#define SYSTEMSTATE_H

#include <QString>
#include "../lh_systemstate.h"

class SystemState
{
    lh_systemstate state_;
    QString dir_binaries;     /**< where the LCDHost executables are stored, may not be writable */
    QString dir_plugins;      /**< where the LCDHost plugins are stored, may not be writable */
    QString dir_data;         /**< where layouts, logs and other data are stored, will be writable */
    QString dir_layout;       /**< directory where the current layout is stored */
    QString layout_file;      /**< file name of the current layout */

public:
    SystemState();
    ~SystemState();

    void init( lh_systemstate* );
    void setLayout( lh_systemstate* );
    void term( lh_systemstate* );
};

#endif // SYSTEMSTATE_H
