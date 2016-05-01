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

#ifndef LIBOBJECT_H
#define LIBOBJECT_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <QList>
#include <QStringList>
#include <QTime>
#include <QBasicTimer>

#include "AppId.h"
#include "AppObject.h"
#include "EventRawInput.h"

#include "lh_plugin.h"

class LibClass;

class LibObject : public QObject
{
    Q_OBJECT

    AppId id_;
    QTime cbtime_; // rate limit timer
    unsigned cbcount_; // rate limit counter
    lh_systemstate state_;
    const void *obj_;
    bool setup_completed_;
    QByteArray name_;
    QByteArray devid_; // will only be set for devices
    QList<LibClass*> classes_;

protected:
    QBasicTimer *timer_; // polling timer
    int notify_mask_;
    int setup_item_count_;
    int setup_item_inputs_;
    lh_setup_item **setup_item_list_;
    QString *setup_extra_str_;
    int *setup_extra_int_;

public:
    LibObject(
        AppId id,
        QString name,
        const void *obj = 0,
        QObject *parent = 0 );
    virtual ~LibObject();

    AppId id() const { return id_; }
    const char *name() const { return name_.constData(); }
    void setId(AppId id) { id_ = id; }
    const lh_systemstate* state() const { return &state_; }

    // plugin object lockers
    virtual const lh_object_calltable *lock();
    virtual void unlock();

    // lh_object_calltable redirectors
    const char * obj_init();
    lh_setup_item ** obj_setup_data();
    void obj_setup_resize( lh_setup_item *item,size_t needed );
    void obj_setup_change( lh_setup_item *item );
    void obj_setup_input( lh_setup_item *item, int flags, int value );
    int obj_polling();
    int obj_notify( int code, void* param );
    const lh_class ** obj_class_list();
    void obj_term();

    virtual QString init();
    virtual int notify(int, void *);
    virtual void term();

    void setObj( const void *obj ) { obj_ = obj; }
    void *obj() const { return const_cast<void*>(obj_); }

    bool isSetupCompleted() const { return setup_completed_; }
    // void setupCompleted(bool b) { setup_completed_ = b; }
    void deviceChanged( int w, int h, int d );

    virtual bool event( QEvent * );

    void eventRawInput( EventRawInput *evt );

    void timerEvent(QTimerEvent *);
    LibClass *getClassById( QString id ) const;
    void loadClassList();
    int classCount() const { return classes_.size(); }

    static void lh_callback( int cb_id, const void *obj, lh_callbackcode code, void *param );
};

#endif // LIBOBJECT_H
