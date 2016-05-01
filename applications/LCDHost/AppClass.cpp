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


#include "AppClass.h"
#include "AppLibrary.h"
#include "LCDHost.h"

AppClass::AppClass(AppLibrary *lib, const QString &id, const QString &path, const QString &name, const QSize &size) :
    QObject(lib),
    seqno_(0),
    name_(name),
    size_(size),
    path_(path.trimmed().split('/', QString::SkipEmptyParts))
{
    if(path_.isEmpty())
        path_.append( lib->name() );
    path_.append(name_);
    setObjectName(id);
}

AppClass::~AppClass()
{
}

AppLibrary *AppClass::parent() const
{
    return qobject_cast<AppLibrary *>(QObject::parent());
}

QString AppClass::generateName()
{
    return QString("%1 [%2]").arg(name_).arg(++seqno_);
}

QVariant AppClass::data( int column, int role )
{
    Q_UNUSED(column);
    if( role == Qt::DisplayRole )
    {
        return this->name_;
    }
    return QVariant();
}

QVariant AppClass::headerData( int section, Qt::Orientation orientation, int role )
{
    if( orientation == Qt::Horizontal)
    {
        if( role == Qt::TextAlignmentRole )
        {
            return Qt::AlignLeft;
        }
        if( role == Qt::DisplayRole )
        {
            switch( section )
            {
            case 0: return QString("Name"); break;
            }
            return QString("Column %1").arg(section);
        }
    }
    return QVariant();
}
