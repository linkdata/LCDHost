/**
  \file     LH_QtPlugin.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011 Johan Lindh

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Link Data Stockholm nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
  */

#ifndef LH_QTPLUGIN_H
#define LH_QTPLUGIN_H

#include <QtGlobal>
#include <QObject>
#include <QStringList>

#include "LH_QtObject.h"

typedef lh_class *(*lh_class_info_t)();
typedef void *(*lh_class_factory_t)(const lh_class *);
class LH_API5PLUGIN_EXPORT LH_QtClassLoader
{
public:
    LH_QtClassLoader *next_;
    lh_class_info_t info_;
    lh_class_factory_t factory_;
    LH_QtClassLoader(LH_QtClassLoader **first_p, lh_class_info_t info, lh_class_factory_t factory) :
        next_(*first_p),
        info_(info),
        factory_(factory)
    {
        *first_p = this;
    }
};

/**
  Base class for Qt-based LCDHost shared libraries.
  As of alpha 17, the LH_QtPlugin object is no longer a global
  C++ object, rather it is created dynamically in lh_create()
  and destroyed in lh_destroy(). These two functions are
  defined when you use the LH_PLUGIN(classname) macro.
  */
class LH_API5PLUGIN_EXPORT LH_QtPlugin : public LH_QtObject
{
    Q_OBJECT

    LH_QtClassLoader **first_p_;
    const lh_class **classlist_;
    lh_object_calltable objtable_;

public:
    LH_QtPlugin();
    ~LH_QtPlugin();
    void setFirstClass(LH_QtClassLoader **first_p) { first_p_ = first_p; }
    virtual const lh_class **class_list();
    const lh_object_calltable * objtable() { return & objtable_; }
    void addClass(LH_QtClassLoader *next);
    int notify( int code, void *param );
};

/**
  This macro creates the required exported functions
  for your LH_QtPlugin descendant.
  */
#define LH_PLUGIN(classname) \
    LH_SIGNATURE(); \
    classname *lh_the_plugin = 0; \
    LH_QtClassLoader *lh_first_class = 0; \
    LH_QtPlugin * lh_plugin() { return static_cast<LH_QtPlugin *>(lh_the_plugin); } \
    extern "C" Q_DECL_EXPORT void * lh_create() \
    { \
        lh_the_plugin = new classname; \
        lh_the_plugin->setFirstClass(&lh_first_class); \
        return reinterpret_cast<void*>(lh_the_plugin); \
    } \
    extern "C" Q_DECL_EXPORT void lh_destroy( void *ref ) { lh_the_plugin = 0; delete reinterpret_cast<classname*>(ref); } \
    extern "C" Q_DECL_EXPORT const lh_object_calltable* lh_get_object_calltable( void *ref ) \
    { \
        if( LH_QtPlugin * p = reinterpret_cast<LH_QtPlugin *>( ref ) ) \
            return p->objtable(); \
        return 0; \
    }
// extern void lh_add_class( lh_class *p, lh_class_factory_t f );
// extern void lh_remove_class( lh_class *p );

#endif // LH_QTPLUGIN_H
