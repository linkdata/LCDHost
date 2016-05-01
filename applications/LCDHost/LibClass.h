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

#ifndef LIBCLASS_H
#define LIBCLASS_H

#include <QString>
#include <QSize>
#include "lh_plugin.h"
#include "LibObject.h"

class LibClass
{
    const lh_class *cls_; // saved so we can send it to obj_new()
    QString id_; // baseName '/' class_name
    QString path_;
    QString name_;
    QSize size_;

public:
    LibClass( const lh_class *class_ptr, LibObject *parent = 0);
    LibClass( const LibClass& other ) : cls_(other.cls_), id_(other.id_),
            path_(other.path_), name_(other.name_), size_(other.size_)
    {
        return;
    }

    QString id() const { return id_; }
    QString path() const { return path_; }
    QString name() const { return name_; }
    QSize size() const { return size_; }
    const lh_object_calltable* objtable() const { return & cls_->objtable; }
    const lh_instance_calltable* table() const { return & cls_->table; }

    void clear() { cls_ = 0; id_.clear(); }

    bool isValid() const
    {
        return (cls_!=NULL) &&
                (cls_->objtable.size == sizeof(lh_object_calltable)) &&
                (cls_->table.size == sizeof(lh_instance_calltable) );
    }

    void * obj_new() { return table()->obj_new( cls_ ); }
    const lh_class *cls() const { return cls_; }

    bool operator==(const LibClass& other) { return id() == other.id(); }
    bool operator==(const lh_class* cls) { return cls ? id().endsWith( QString("/") + QString::fromUtf8(cls->id) ) : false; }
};

#endif // LIBCLASS_H
