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
#include <QCoreApplication>
#include <QThread>

#include "LCDHost.h"
#include "AppRawInput.h"
#include "LibObject.h"
#include "LibDevice.h"
#include "LibClass.h"
#include "LibInstance.h"
#include "LibDeviceData.h"

#include "EventSetupItem.h"
#include "EventSetupItemEnum.h"
#include "EventSetupComplete.h"
#include "EventSetupItemRefresh.h"
#include "EventNotify.h"
#include "EventCallback.h"
#include "EventObjectCreated.h"
#include "EventDeviceChanged.h"
#include "EventRequestVisibility.h"
#include "EventLayoutLoad.h"
#include "EventLayoutChanged.h"
#include "EventClassLoaded.h"
#include "EventClassRemoved.h"
#include "EventRequestRender.h"
#include "EventLoadClassList.h"
#include "EventRequestPolling.h"
#include "EventRequestUnload.h"
#include "EventObjectDestroy.h"

#ifdef Q_OS_WIN
#include <windows.h> // for Sleep
#endif
void qSleep(int ms)
{
#ifdef Q_OS_WIN
  Sleep(uint(ms));
#else
  struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
  nanosleep(&ts, NULL);
#endif
}

void LibObject::lh_callback( int cb_id, const void *obj, lh_callbackcode code, void *param )
{
  AppId id(cb_id);
  int count = 0;
  int elapsed = 0;

  // rate limiter
  if( LibObject *lo = id.lock() )
  {
    count = ++ lo->cbcount_;
    elapsed = lo->cbtime_.elapsed();
    id.unlock();
  }

  if( count > 1000 )
  {
    if( elapsed < 1000 )
    {
      qWarning() << id << "is out of control, rate limiting it";
      qSleep( 1000 - elapsed );
    }
    if( LibObject *lo = id.lock() )
    {
      lo->cbcount_ = 0;
      lo->cbtime_.start();
      id.unlock();
    }
    else qWarning() << id << "failed to reset rate limiter";
  }

  switch( code )
  {
    case lh_cb_unload:
      id.postLibEvent( new EventRequestUnload( param ? QString::fromUtf8((const char *)param) : QString()) );
      break;

    case lh_cb_polling:
      id.postLibEvent( new EventRequestPolling() );
      break;

    case lh_cb_setup_refresh:
      if(lh_setup_item *lsi = (lh_setup_item *)param)
      {
        id.postAppEvent(new EventSetupItemRefresh(lsi));
        lsi->flags &= ~LH_FLAG_NEEDREFRESH;
      }
      break;

    case lh_cb_setup_rebuild:
      id.postLibEvent( new EventSetupItemEnum() );
      return;

    case lh_cb_notify:
      id.postLibEvent( new EventNotify( 0, 0 ) );
      break;

    case lh_cb_log:
      if( param ) qDebug() << id << QString::fromUtf8( (const char *) param );
      break;

    case lh_cb_render:
      id.postAppEvent( new EventRequestRender(id) );
      break;

    case lh_cb_sethidden:
      if( param )
        id.postAppEvent( new EventRequestVisibility( id, !(*(int*)param) ) );
      break;

    case lh_cb_input:
      if( param )
      {
        lh_device_input *di = (lh_device_input*) param;
        QCoreApplication::postEvent( AppRawInput::instance(), new EventRawInput(
                                       id,
                                       di->devid,
                                       di->control,
                                       di->item,
                                       di->value,
                                       di->flags ), Qt::HighEventPriority+1 );
      }
      break;

    case lh_cb_load_layout:
      id.postAppEvent( new EventLayoutLoad( QString::fromUtf8( (const char*) param ) ) );
      break;

    case lh_cb_class_refresh:
      id.postLibEvent( new EventLoadClassList() );
      break;

      // handled directly in callback, as it doesn't require a LibObject
    case lh_cb_utf8_to_local8bit:
      if( param )
      {
        QString str = QString::fromUtf8( (const char*)param );
        QByteArray ary = str.toLocal8Bit();
        if( ary.size() <= (int) strlen((const char*)param) )
          memcpy( param, ary.constData(), ary.size()+1 );
      }
      return;

    case lh_cb_arrive:
      LibDevice::arrive(id, obj, (lh_device*) param);
      return;

    case lh_cb_leave:
      LibDevice::leave(id, obj, (lh_device*) param);
      return;

    case lh_cb_unused:
      Q_ASSERT(0);
      return;
  }

  return;
}

LibObject::LibObject( AppId id, QString name, const void *obj, QObject *parent ) :
  QObject(parent),
  id_(id),
  cbcount_(0),
  obj_(obj),
  setup_completed_(false),
  timer_(0),
  notify_mask_(~0),
  setup_item_count_(0),
  setup_item_inputs_(0),
  setup_item_list_(0),
  setup_extra_str_(0),
  setup_extra_int_(0)
{
  Q_ASSERT( id_.isValid() );

  cbtime_.start();
  memset( &state_, 0, sizeof(state_) );
  AppState::instance()->stateInit( &state_ );

  if( !name.isEmpty() )
  {
    name.replace('/','-');
    name_ = name.toUtf8();
    setObjectName( name );
  }
}

LibObject::~LibObject()
{
  if( id().isValid()  )
    qWarning() << metaObject()->className() << objectName() << "has not been terminated";
  if( timer_ )
  {
    delete timer_;
    timer_ = NULL;
  }
  if( setup_extra_str_ )
  {
    delete[] setup_extra_str_;
    setup_extra_str_ = NULL;
  }
  if( setup_extra_int_ )
  {
    delete[] setup_extra_int_;
    setup_extra_int_ = NULL;
  }
  setup_item_count_ = 0;
  setup_item_inputs_ = 0;
  setup_item_list_ = NULL;
  AppState::instance()->stateTerm( &state_ );
  return;
}

const lh_object_calltable *LibObject::lock()
{
  Q_ASSERT(0);
  return 0;
}

void LibObject::unlock()
{
}

const char * LibObject::obj_init()
{
  const char *retv = "obj_init() can't lock object";
  if( const lh_object_calltable *objtable=lock() )
  {
    if( objtable->obj_init )
      retv = objtable->obj_init( obj(), lh_callback, id().toInt(), name(), state() );
    else
      retv = "missing obj_init()";
    unlock();
  }
  return retv;
}

lh_setup_item ** LibObject::obj_setup_data()
{
  lh_setup_item **retv = 0;
  Q_ASSERT( id().isValid() );
  if( const lh_object_calltable *objtable=lock() )
  {
    if( objtable->obj_setup_data )
      retv = objtable->obj_setup_data( obj() );
    unlock();
  }
  return retv;
}

void LibObject::obj_setup_resize( lh_setup_item *item, size_t needed )
{
  Q_ASSERT( id().isValid() );
  if( const lh_object_calltable *objtable=lock() )
  {
    if( objtable->obj_setup_resize )
      objtable->obj_setup_resize( obj(), item, needed );
    unlock();
  }
  return;
}

void LibObject::obj_setup_change( lh_setup_item *item )
{
  Q_ASSERT( id().isValid() );
  if( const lh_object_calltable *objtable=lock() )
  {
    if( objtable->obj_setup_change )
      objtable->obj_setup_change( obj(), item );
    else
      Q_ASSERT(!"obj_setup_change is not set");
    unlock();
  }
  else
    Q_ASSERT(!"LibObject::lock() failed");
  return;
}

void LibObject::obj_setup_input( lh_setup_item *item, int flags, int value )
{
  Q_ASSERT( id().isValid() );
  if( const lh_object_calltable *objtable=lock() )
  {
    if( objtable->obj_setup_input )
      objtable->obj_setup_input( obj(), item, flags, value );
    unlock();
  }
  return;
}

int LibObject::obj_polling()
{
  int retv = 0;
  Q_ASSERT( id().isValid() );
  if( const lh_object_calltable *objtable=lock() )
  {
    if( objtable->obj_polling )
      retv = objtable->obj_polling( obj() );
    unlock();
  }
  if( retv && retv < 16 ) retv = 16;
  return retv;
}

int LibObject::obj_notify( int code, void* param )
{
  int retv = 0;
  Q_ASSERT( id().isValid() );
  if( const lh_object_calltable *objtable=lock() )
  {
    if( objtable->obj_notify )
      retv = objtable->obj_notify( obj(), code, param );
    unlock();
  }
  else
  {
    qCritical() << metaObject()->className() << objectName()
                << "LibObject::obj_notify(" << code << ", " << (qptrdiff) param
                <<"): failed to lock()";
  }
  return retv;
}

const lh_class ** LibObject::obj_class_list()
{
  const lh_class **retv = 0;
  Q_ASSERT( id().isValid() );
  if( const lh_object_calltable *objtable=lock() )
  {
    if( objtable->obj_class_list )
      retv = objtable->obj_class_list( obj() );
    unlock();
  }
  return retv;
}

void LibObject::obj_term()
{
  if( const lh_object_calltable *objtable=lock() )
  {
    if( objtable->obj_term )
      objtable->obj_term( obj() );
    unlock();
    return;
  }
  qDebug("LibObject::obj_term(): lock() failed for %s", name_.constData());
  return;
}

QString LibObject::init()
{
#ifndef QT_NO_DEBUG
  if( id().hasLibObject() )
    qWarning() << metaObject()->className() << objectName() << "already initialized";
#endif
  id_.setLibObject(this);
  if(const char *retv = obj_init())
  {
    id_.setLibObject(0);
    return QString::fromUtf8(retv);
  }
  id_.postAppEvent(new EventObjectCreated(id_));
  return QString();
}

int LibObject::notify(int code, void *param)
{
  Q_UNUSED(param);

  if (AppState* as = AppState::instance()) {
    if( code & LH_NOTE_SECOND )
    {
      as->stateRefreshSecond( &state_ );
    }
    if( code & LH_NOTE_MEM )
    {
      as->stateRefreshMemory( &state_ );
    }
    if( code & LH_NOTE_NET )
    {
      as->stateRefreshNetwork( &state_ );
    }
    if( code & LH_NOTE_CPU )
    {
      as->stateRefreshCPU( &state_ );
    }
  }

  return obj_notify( code, param );
  //            LH_NOTE_CPU|LH_NOTE_NET|LH_NOTE_MEM|LH_NOTE_SECOND;
}

void LibObject::term()
{
  setup_completed_ = false;
  if( timer_ ) delete timer_;
  timer_ = NULL;

  while( !classes_.isEmpty() )
  {
    LibClass *lib_class = classes_.takeFirst();
    id().postAppEvent( new EventClassRemoved( lib_class->id() ) );
    delete lib_class;
  }

  if( id().isValid() )
  {
    id_.setLibObject(0);
    id_.clear();
    obj_term();
  }
  else
    qWarning() << "LibObject::term()" << metaObject()->className() << objectName() << "has no id";
}

void LibObject::eventRawInput( EventRawInput *evt )
{
  int i;
  lh_setup_item *psi;

  for( psi=setup_item_list_[i=0]; psi; psi=setup_item_list_[++i] )
  {
    if( psi->type == lh_type_string_inputstate || psi->type == lh_type_string_inputvalue )
    {
      if( setup_extra_str_[i] == evt->control )
      {
        obj_setup_input( psi, evt->flags, evt->value );
      }
    }
  }
}

void LibObject::loadClassList()
{
  const lh_class **a_class_list;
  const lh_class *a_class;
  QList<LibClass *> new_list;

  // build the new class list
  a_class_list = obj_class_list();

  if( a_class_list )
  {
    a_class = *a_class_list;
    while( a_class )
    {
      if( a_class->size == sizeof(lh_class) && a_class->name )
      {
        new_list.append( new LibClass( a_class, this ) );
        a_class_list++;
        a_class = *a_class_list;
      }
      else
      {
        qWarning() << id() << ": illegal class list entry";
        break;
      }
    }
  }

  if( lh_log_load )
    qDebug() << QString("<span style=\"background-color: #f0f0f0;\">Classes</span> <tt>%1</tt> lh_class_list: %2 items")
                .arg( objectName() )
                .arg( a_class_list ? new_list.size() : -1 )
                ;

  // send add for all those in new not in old
  foreach( LibClass *new_class, new_list )
  {
    bool found = false;
    foreach( LibClass *old_class, classes_ )
    {
      if( new_class->id() == old_class->id() )
      {
        found = true;
        break;
      }
    }
    if( !found )
    {
      id().postAppEvent( new EventClassLoaded(
                           new_class->id(),
                           new_class->path(),
                           new_class->name(),
                           new_class->size()
                           ) );
    }
  }

  // send remove for all those that are in old but not in new
  foreach( LibClass *old_class, classes_ )
  {
    bool found = false;
    foreach( LibClass *new_class, new_list )
    {
      if( new_class->id() == old_class->id() )
      {
        found = true;
        break;
      }
    }
    if( !found )
    {
      classes_.removeAll( old_class );
      id().postAppEvent( new EventClassRemoved( old_class->id() ) );
      delete old_class;
    }
  }

  // delete the remaining stuff in the old list (they're duplicates)
  // and then switch to the new list
  while( !classes_.isEmpty() ) delete classes_.takeFirst();
  classes_ = new_list;

  return;
}

LibClass *LibObject::getClassById( QString id ) const
{
  for( QList<LibClass*>::const_iterator ci=classes_.constBegin(); ci!=classes_.constEnd(); ++ci )
    if( (*ci)->id() == id ) return (*ci);
  return NULL;
}

void LibObject::timerEvent(QTimerEvent *event)
{
  if( timer_ && event->timerId() == timer_->timerId() )
  {
    obj_polling();
    return;
  }
  QObject::timerEvent(event);
}

void LibObject::deviceChanged( int w, int h, int d )
{
  state_.dev_width = w;
  state_.dev_height = h;
  state_.dev_depth = d;
  state_.dev_fps = 0;
  foreach( QObject *o, children() )
  {
    LibObject *lib_obj = qobject_cast<LibObject *>(o);
    if( lib_obj ) lib_obj->deviceChanged(w,h,d);
  }
  LibInstance *lib_inst = qobject_cast<LibInstance *>(this);
  if( lib_inst ) lib_inst->requestRender();
}

bool LibObject::event( QEvent *event )
{
  if( event->type() < QEvent::User || event->type() > QEvent::MaxUser ) return QObject::event(event);

  if( id().isEmpty() )
  {
    qDebug() << "LibObject::event() : dead event" << EventBase::name(event->type()) << "for" << objectName();
    return true;
  }

  if( event->type() == EventSetupItemRefresh::type() )
  {
    // Sent by AppObject when an AppSetupItem changes and it wants
    // to propagate the changes to the plugin.
    EventSetupItemRefresh *e = static_cast<EventSetupItemRefresh*>(event);
    int i;
    lh_setup_item *psi;

    if( setup_item_list_ )
    {
      for( psi=setup_item_list_[i=0]; psi; psi=setup_item_list_[++i] )
      {
        if( e->lsi.name() == psi->name )
        {
          size_t required = e->lsi.fillSize( psi );

          if( required > psi->param.size )
            obj_setup_resize( psi, required );


          if( e->lsi.fillItem( psi ) )
          {
            setup_extra_str_[i] = e->lsi.extraStr();
            setup_extra_int_[i] = e->lsi.extraInt();
            obj_setup_change( psi );
          }

          return true;
        }
      }
#ifndef QT_NO_DEBUG
      qDebug() << "LibObject::event(EventSetupItemRefresh): no setup item"
               << id().toString()
               << e->lsi.name();
#endif
    }
    return true;
  }

  if( event->type() == EventLoadClassList::type() )
  {
    loadClassList();
    return true;
  }

  if( event->type() == EventRequestUnload::type() )
  {
    EventRequestUnload *e = static_cast<EventRequestUnload*>(event);
    qWarning() << name() << "requested unload" << e->message;
    thread()->exit(0);
    return true;
  }

  if( event->type() == EventRequestPolling::type() )
  {
    int ms;
    ms = obj_polling();
    if( ms > 0 )
    {
      if( timer_ == NULL ) timer_ = new QBasicTimer();
      timer_->start(ms,this);
    }
    else
    {
      if( timer_ ) delete timer_;
      timer_ = NULL;
    }
    return true;
  }

  if( event->type() == EventNotify::type() )
  {
    EventNotify *e = static_cast<EventNotify*>(event);
    notify( e->note, e->param );
    // if( e->note && !(e->note & notify_mask_) ) return true;
    // notify_mask_ = obj_notify( e->note, e->param );
    return true;
  }

  if( event->type() == EventSetupItemEnum::type() )
  {
    // Go through our list of setup items as provided by the plugin,
    // and send them to the AppObject. This is always done, since only
    // the plugin knows the true list of items. These will also provide
    // defaults in case the AppObject doesn't have them, and check if
    // the items can provide help info.
    lh_setup_item *p;
    int i;

    setup_completed_ = false;
    setup_item_count_ = 0;
    setup_item_inputs_ = 0;
    setup_item_list_ = obj_setup_data();

    if( setup_item_list_ )
    {
      for( p=setup_item_list_[i=0]; p; p=setup_item_list_[++i] )
      {
        if( p->type <= lh_type_none || p->type >= lh_type_last )
        {
          qWarning() << QString("%1: invalid setup_item_list[%2] (0x%3) type=%4 flags=0x%5")
                        .arg(id().toString())
                        .arg(setup_item_count_)
                        .arg((quintptr)p,1,16)
                        .arg(p->type)
                        .arg(p->flags,1,16);
          break;
        }
        if( p->type == lh_type_string_inputstate || p->type == lh_type_string_inputvalue )
          setup_item_inputs_ ++;
        setup_item_count_ ++;
      }

      setup_extra_str_ = new QString[setup_item_count_];
      setup_extra_int_ = new int[setup_item_count_];

      for( i=0; i<setup_item_count_; i++ )
      {
        setup_extra_str_[i].clear();
        setup_extra_int_[i] = 0;
        id().postAppEvent( new EventSetupItem( setup_item_list_[i] ) );
      }
    }

    id().postAppEvent( new EventSetupItemEnum() );
    return true;
  }

  if( event->type() == EventRawInput::type() )
  {
    // Sent from application when a raw input event occurs
    EventRawInput *e = static_cast<EventRawInput *>(event);
    if( isSetupCompleted() ) eventRawInput(e);
    return true;
  }

  if( event->type() == EventObjectDestroy::type() )
  {
    term();
    deleteLater();
    return true;
  }

  if( event->type() == EventSetupComplete::type() )
  {
    if( !isSetupCompleted() )
    {
      setup_completed_ = true;
      QCoreApplication::postEvent( this, new EventRequestPolling() );
      QCoreApplication::postEvent( this, new EventNotify(0,0) );
      id().postAppEvent( new EventSetupComplete(id()) );
    }
    else
    {
      qDebug() << metaObject()->className() << id()
               << "Duplicate EventSetupComplete";
    }
    return true;
  }

  if( event->type() == EventDeviceChanged::type() )
  {
    EventDeviceChanged *e = static_cast<EventDeviceChanged*>(event);
    deviceChanged(e->size.width(),e->size.height(),e->depth);
    return true;
  }

  if( event->type() == EventLayoutChanged::type() )
  {
    if (AppState* as = AppState::instance())
      as->stateRefreshLayout( &state_ );
    return true;
  }

  return false;
}
