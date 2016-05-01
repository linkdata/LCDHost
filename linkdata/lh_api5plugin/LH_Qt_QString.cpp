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

#include "LH_Qt_QString.h"
#include <QDebug>

void LH_Qt_QString::setup_resize( size_t needed )
{
    array_.resize(needed);
    item_.param.size = array_.capacity();
    item_.data.s = array_.data();
    return;
}

void LH_Qt_QString::setup_change()
{
    Q_ASSERT( item_.data.s == array_.constData() );
    str_ = QString::fromUtf8( item_.data.s );
    emit change( str_ );
    LH_QtSetupItem::setup_change();
    return;
}

void LH_Qt_QString::setValue(QString s)
{
    Q_ASSERT( item_.data.s == array_.constData() );
    if( s != str_ )
    {
        str_ = s;
        array_ = str_.toUtf8();
        item_.param.size = array_.capacity();
        item_.data.s = array_.data();
        refresh();
        emit set();
    }
}
