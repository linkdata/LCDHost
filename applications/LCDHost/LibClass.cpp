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

#include "LibLibrary.h"
#include "LibClass.h"

LibClass::LibClass( const lh_class *class_ptr, LibObject *parent ) : cls_(class_ptr)
{
    Q_ASSERT( class_ptr->size == sizeof(lh_class) );
    Q_ASSERT( class_ptr->objtable.size == sizeof(lh_object_calltable) );
    Q_ASSERT( class_ptr->table.size == sizeof(lh_instance_calltable) );
    if( parent ) { id_ = parent->objectName(); id_.append('/'); }
    id_.append(QString::fromUtf8(class_ptr->id));
    if( class_ptr->path ) path_ = QString::fromUtf8( class_ptr->path );
    if( class_ptr->name ) name_ = QString::fromUtf8( class_ptr->name );
    size_.setHeight( class_ptr->height < 1 ? 24 : class_ptr->height );
    size_.setWidth( class_ptr->width < 1 ? 24 : class_ptr->width );
    return;
}
