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

#ifndef LIBINSTANCE_H
#define LIBINSTANCE_H

#include <QObject>
#include <QImage>

#include "LibObject.h"
#include "LibClass.h"
#include "LibLibrary.h"

class LibInstance : public LibObject
{
    Q_OBJECT

    LibClass class_;

public:
    LibInstance( AppId id, const LibClass& cls, QString name, const void *obj, LibLibrary *parent );
    ~LibInstance();

    virtual QString init();
    virtual void term();

    virtual const lh_object_calltable *lock() { return class_.objtable(); }
    virtual void unlock() {}

    bool isValid() const { return obj() ? true : false; }
    LibLibrary *libLibrary() const { return static_cast<LibLibrary*>(parent()); }
    const LibClass & libClass() const { return class_; }

    void obj_prerender() { if( class_.table()->obj_prerender ) class_.table()->obj_prerender(obj()); }
    int obj_width(int h=-1) { return class_.table()->obj_width ? class_.table()->obj_width(obj(),h) : -1; }
    int obj_height(int w=-1) { return class_.table()->obj_height ? class_.table()->obj_height(obj(),w) : -1; }
    const lh_blob * obj_render_blob(int w,int h) { return class_.table()->obj_render_blob ? class_.table()->obj_render_blob(obj(),w,h) : NULL; }
    QImage * obj_render_qimage(int w,int h) { return static_cast<QImage*>( class_.table()->obj_render_qimage ? class_.table()->obj_render_qimage(obj(),w,h) : NULL ); }
    void obj_delete() { if( class_.table()->obj_delete ) class_.table()->obj_delete(obj()); }

    int width() const { return class_.size().width(); }
    int height() const { return class_.size().height(); }

    void eventRender( int w, int w_method, int w_mod, int h, int h_method, int h_mod );
    void requestRender();

    bool event( QEvent *event );
};

#endif // LIBINSTANCE_H
