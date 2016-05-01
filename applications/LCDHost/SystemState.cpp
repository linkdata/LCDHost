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


#include <QCoreApplication>
#include <QDesktopServices>
#include <QFileInfo>
#include <QSettings>

#include "SystemState.h"

SystemState::SystemState()
{
    QSettings settings;

    Q_ASSERT( NULL == 0 );
    memset( &state_, 0, sizeof(state_) );
    state_.size = sizeof(state_);

    dir_binaries = QFileInfo( QCoreApplication::applicationFilePath() ).canonicalPath();
    if( !dir_binaries.endsWith('/') ) dir_binaries.append('/');

#ifdef Q_WS_MAC
    dir_plugins = QFileInfo( dir_binaries + "../PlugIns" ).canonicalFilePath();
    if( !dir_plugins.endsWith('/') ) dir_plugins.append('/');
#else
    dir_plugins = dir_binaries + "plugins/";
#endif

    QString defaultdata = QFileInfo(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/LCDHost/" ).canonicalFilePath();
    dir_data = QFileInfo( settings.value( "installPath", defaultdata ).toString() ).canonicalFilePath();
    if( !dir_data.endsWith('/') ) dir_data.append('/');

    return;
}
