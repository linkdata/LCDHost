/**
  \file     LH_QtInstance.h
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

#ifndef LH_QTINSTANCE_H
#define LH_QTINSTANCE_H

#include <QtGlobal>
#include <QImage>

#include "LH_QtPlugin.h"
#include "LH_QtObject.h"
#include "LH_QtPlugin.h"

/**
  Base class for LCDHost plugin classes using Qt. For normal use, the macro
  LH_PLUGIN_CLASS(classname) will export the class from the implementation
  file (not from the header file!).
  */
class LH_API5PLUGIN_EXPORT LH_QtInstance : public LH_QtObject
{
    Q_OBJECT

    QImage *image_;

public:
    explicit LH_QtInstance(LH_QtObject *parent = 0);

    virtual void term();

    bool hasImage() const { return image_ && !image_->isNull(); }
    QImage *image() const { return image_; }
    QImage *initImage(int w, int h);

    virtual void prerender() {}
    virtual int width( int ) { return -1; }
    virtual int height( int ) { return -1; }
    virtual lh_blob *render_blob( int, int ) { return NULL; }
    virtual QImage *render_qimage( int, int ) { return NULL; }

    static void build_instance_calltable( lh_instance_calltable *ct, lh_class_factory_t cf );
    // static const lh_class **auto_class_list();

    /** You MUST reimplement this in your classes if you use the class loader and macros below */
    static lh_class *classInfo() { Q_ASSERT(!"classInfo() not reimplemented"); return NULL; }
};

/**
  This macro creates the required functions and object to allow
  automatic registration of layout classes. Note that using this
  macro requires a static classInfo() method that returns a
  statically allocated lh_class structure pointer.
  */
#define LH_PLUGIN_CLASS(classname)  \
    extern LH_QtClassLoader *lh_first_class; \
    classname *_lh_##classname##_factory(const lh_class *) { return new classname; } \
    lh_class *_lh_##classname##_info() { return classname::classInfo(); } \
    LH_QtClassLoader _lh_##classname##_loader( &lh_first_class, _lh_##classname##_info, reinterpret_cast<lh_class_factory_t>(_lh_##classname##_factory) );

#endif // LH_QTINSTANCE_H
