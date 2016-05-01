/**
  \file     LH_QtObject.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011, Johan Lindh

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

#ifndef LH_QTOBJECT_H
#define LH_QTOBJECT_H

#include <QObject>
#include <QVector>
#include "lh_plugin.h"

#if defined(LH_API5PLUGIN_LIBRARY)
# define LH_API5PLUGIN_EXPORT Q_DECL_EXPORT
#else
# define LH_API5PLUGIN_EXPORT Q_DECL_IMPORT
#endif

class LH_QtPlugin;
LH_QtPlugin *lh_plugin();

/**
  Base class for Qt-based LCDHost plugin objects,
  such as plugins, layout class instances or devices.
  */
class LH_API5PLUGIN_EXPORT LH_QtObject : public QObject
{
    Q_OBJECT

    // static LH_QtPlugin *plugin_;

    lh_callback_t cb_;
    int cb_id_;
    const lh_systemstate *state_;
    QVector<lh_setup_item*> setup_item_vector_;

public:
    // static void set_plugin( LH_QtPlugin *p ) { plugin_ = p; }
    // static LH_QtPlugin *plugin() { return lh_plugin(); }
    static void build_object_calltable( lh_object_calltable *ct );

    LH_QtObject( LH_QtObject *parent = 0);

    LH_QtObject *parent() const { return static_cast<LH_QtObject *>( QObject::parent() ); }

    void callback( lh_callbackcode_t code, void *param ) const;
    bool callable() const { return cb_ != 0; }

    // These implement the lh_object_callback functions. If you reimplement them, make
    // sure to call the base class copy of them and preserve or modify the return value
    // (if any) to suit.
    virtual int polling();
    virtual int notify( int code, void *param );

    virtual const char *init( lh_callback_t cb, int cb_id, const char *name, const lh_systemstate* state );
    virtual lh_setup_item **setup_data();
    virtual void setup_resize( lh_setup_item *item, size_t needed );
    virtual void setup_change( lh_setup_item *item );
    virtual void setup_input( lh_setup_item *item, int flags, int value );
    virtual const lh_class **class_list();
    virtual void term();

    // You can use these two instead of init() and term(), that way you won't need to
    // pass on the parameters that init() takes to the ancestor. init() will call
    // userInit() when it's done and term() will call userTerm() before it does
    // it's work.
    virtual const char *userInit() { return 0; }
    virtual void userTerm() {}

    // Convenience wrappers
    void show() const { int b = 0; callback( lh_cb_sethidden, (void*)&b ); }
    void hide() const { int b = 1; callback( lh_cb_sethidden, (void*)&b ); }
    void setVisible( bool b ) const { int notb = !b; callback( lh_cb_sethidden, (void*)&notb ); }

    const lh_systemstate* state() const { return state_; }


signals:
    void initialized();

public slots:
    void requestRebuild() const { callback( lh_cb_setup_rebuild, NULL ); } // call after adding or removing setup items!
    void requestRender() const { callback( lh_cb_render, NULL ); }
    void requestPolling() const { callback( lh_cb_polling, NULL ); }
};

#endif // LH_QTOBJECT_H
