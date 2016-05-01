/**
  \file     LH_QtInstance.cpp
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2010 Johan Lindh

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
#include "LH_QtInstance.h"

// LH_QtClassLoader *LH_QtClassLoader::first_ = NULL;
// static const lh_class **classlist_ = NULL;
// static QList<lh_layout_class> *manual_list_ = NULL;

#define RECAST(obj) reinterpret_cast<LH_QtInstance*>(obj)
static void obj_prerender(void *obj) { RECAST(obj)->prerender(); }
static int obj_width(void *obj,int h) { return RECAST(obj)->width(h); }
static int obj_height(void *obj,int w) { return RECAST(obj)->height(w); }
static const lh_blob * obj_render_blob(void *obj,int w,int h) { return RECAST(obj)->render_blob(w,h); }
static void * obj_render_qimage(void *obj,int w,int h) { return RECAST(obj)->render_qimage(w,h); }
static void obj_delete(void *obj) { delete RECAST(obj); }

void LH_QtInstance::build_instance_calltable( lh_instance_calltable *ct, lh_class_factory_t cf )
{
    if( ct )
    {
        ct->size = sizeof(lh_instance_calltable);
        ct->obj_new = cf;
        ct->obj_prerender = obj_prerender;
        ct->obj_width = obj_width;
        ct->obj_height = obj_height;
        ct->obj_render_blob = obj_render_blob;
        ct->obj_render_qimage = obj_render_qimage;
        ct->obj_delete = obj_delete;
    }
    return;
}

LH_QtInstance::LH_QtInstance(LH_QtObject *parent) :
    LH_QtObject(parent),
    image_(0)
{}

void LH_QtInstance::term()
{
    if( image_ )
    {
        delete image_;
        image_ = NULL;
    }
    LH_QtObject::term();
    return;
}

/**
  Basic QImage handling. Call this at the start of
  your render_qimage to ensure that \c image_ is
  of the correct size. Always check the return
  value, and return NULL if the return value is
  NULL.
  */
QImage *LH_QtInstance::initImage(int w, int h)
{
    if( image_ && ( w != image_->width() || h != image_->height() ) )
    {
        delete image_;
        image_ = NULL;
    }
    if(!image_ && w > 0 && h > 0)
    {
        QImage *img = new QImage(w, h, QImage::Format_ARGB32_Premultiplied);
        if(img->isNull())
            delete img;
        else
            image_ = img;
    }
    return image_;
}

