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


#include <stdlib.h>

#include <QtDebug>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

#include "AppState.h"
#include "AppObject.h"
#include "AppLibrary.h"
#include "../lh_logger/LH_Logger.h"

QAtomicPointer<AppState> AppState::instance_ = 0;

AppState::AppState(QObject* parent)
  : QObject(parent)
  , dev_size_(0, 0)
  , dev_depth_(0)
  , dev_fps_(0)
  , run_time_(0)
  , run_idle_(0)
  , net_max_in_(1024 * 1024 * 100 / 8)
  , net_max_out_(1024 * 1024 * 100 / 8)
  , net_cur_in_(0)
  , net_cur_out_(0)
  , cpu_count_(0)
  , cpu_load_(0)
  , cpu_self_now_(0)
  , cpu_self_avg_(0)
  // , cpu_cores_curr_(0)
  // , cpu_cores_prev_(0)
  , cpuLastSelfUsage_(0)
  , cpuLastSelfTime_(0)
  , layout_rps_(0)
  , fps_(0)
  , rps_(0)
{
  if(instance_.testAndSetOrdered(0, this))
  {
    setObjectName("AppState");
    memset( &cpu_cores_curr_, 0, sizeof(cpu_cores_curr_) );
    memset( &cpu_cores_prev_, 0, sizeof(cpu_cores_prev_) );
    memset( &net_data_, 0, sizeof(net_data_) );
    memset( &netLastSecond_, 0, sizeof(netLastSecond_) );
    memset( &mem_data_, 0, sizeof(mem_data_) );

    dir_binaries_ = QCoreApplication::applicationDirPath();
    if( !dir_binaries_.endsWith('/') ) dir_binaries_.append('/');

#ifdef Q_OS_MAC
    dir_plugins_ = QDir::cleanPath(dir_binaries_ + "../PlugIns");
#else
    dir_plugins_ = dir_binaries_;
#endif
    if( !dir_plugins_.endsWith('/') ) dir_plugins_.append('/');
    dir_data_ = lh_data_dir();
    if( !dir_data_.endsWith('/') ) dir_data_.append('/');
    QDir().mkpath(dir_data());
  }
}

AppState::~AppState()
{
  if(instance_.testAndSetOrdered(this, 0)) {
    /* no-op */
  }
}

AppState *AppState::instance()
{
  return instance_.load();
}

void AppState::setLayout( QString path, QString file )
{
  QFileInfo fi( QDir(path), file );
  dir_layout_ = fi.absolutePath();
  if( !dir_layout_.endsWith('/') ) dir_layout_.append('/');
  layout_file_ = fi.fileName();
}

char *AppState::filenameToUtf8( QString s )
{
  QByteArray array = s.toUtf8();
  char *retv = new char[array.size()+1];
  memcpy( retv, array.constData(), array.size()+1 );
  return retv;
}

void AppState::updateCPU()
{
  cpu_count_ = cpuEngine_.count();
  cpuEngine_.sample(cpu_count_, cpu_cores_curr_);
  return;
}

void AppState::updateMemory()
{
  memEngine_.sample( &mem_data_ );
}

void AppState::updateNetwork( QString filter )
{
  netEngine_.sample( &net_data_, filter );
}

/**
  Given two lh_cpudata, calculate the average CPU load between them.
  Return the load in hundredths of a percent (0...10000).
  */
int lh_cpuload(const lh_cpudata* from, const lh_cpudata* to, qint64 timeunit)
{
  if (!from || !to || !to->when || !from->when || to->when == from->when)
    return 0;
  if (from->when > to->when)
    qSwap(from, to);
  qreal idle_delta = (qreal)(to->idle - from->idle);
  qreal work_delta = (qreal)(to->work - from->work);
  qreal load = 1.0 - idle_delta / (idle_delta + work_delta);
  if (timeunit > 0)
    load *= (qreal)timeunit / (qreal)(to->when - from->when);
  return (int) (10000.0 * load);
}

void AppState::updateSecond()
{
  run_time_ ++;

  if(cpu_count_)
  {
    lh_cpudata prev, curr;
    prev.when = cpu_cores_prev_[0].when;
    prev.idle = prev.work = 0;
    curr.when = cpu_cores_curr_[0].when;
    curr.idle = curr.work = 0;
    cpuEngine_.sample(cpu_count_, cpu_cores_curr_);
    for(int n = 0; n < cpu_count_; ++n) {
      prev.idle += cpu_cores_prev_[n].idle;
      prev.work += cpu_cores_prev_[n].work;
      curr.idle += cpu_cores_curr_[n].idle;
      curr.work += cpu_cores_curr_[n].work;
    }
    cpu_load_ = lh_cpuload(&prev, &curr, 1000);
    memcpy(&cpu_cores_prev_, &cpu_cores_curr_, sizeof(lh_cpudata) * cpu_count_);
  }

  qreal newSelfUsage = cpuEngine_.selfusage();
  qreal newSelfTime = run_time_;
  cpu_self_now_ = qRound( (newSelfUsage - cpuLastSelfUsage_) * 10000.0 / (newSelfTime - cpuLastSelfTime_) );
  cpu_self_avg_ = qRound( newSelfUsage * 10000.0 / newSelfTime );
  cpuLastSelfUsage_ = newSelfUsage;
  cpuLastSelfTime_ = newSelfTime;

  if( netLastSecond_.device == net_data_.device && (netLastSecond_.in || netLastSecond_.out) )
  {
    qint64 factor = (net_data_.when - netLastSecond_.when);
    net_cur_in_ = (net_data_.in - netLastSecond_.in) * factor / Q_INT64_C(1000);
    net_cur_out_ = (net_data_.out - netLastSecond_.out) * factor / Q_INT64_C(1000);
  }
  else
  {
    net_cur_in_ = 0;
    net_cur_out_ = 0;
  }
  memcpy(&netLastSecond_, &net_data_, sizeof(lh_netdata));

  dev_fps_ = fps_; fps_ = 0;
  layout_rps_ = rps_; rps_ = 0;

  // qDebug("AppState::updateSecond(): QObject count: %d", (qApp->findChildren<QObject *>()).size());

  return;
}

void AppState::stateInit( lh_systemstate* state )
{
  if( state == NULL ) return;

  state->size = sizeof(lh_systemstate);

  state->dev_width = dev_size_.width();
  state->dev_height = dev_size_.height();
  state->dev_depth = dev_depth_;
  state->dev_fps = dev_fps_;

  state->run_time = run_time_;
  state->run_idle = 0;

  state->net_max_in = net_max_in_;
  state->net_max_out = net_max_out_;
  state->net_cur_in = net_cur_in_;
  state->net_cur_out = net_cur_out_;
  memcpy( &state->net_data, &net_data_, sizeof(state->net_data) );

  memcpy( &state->mem_data, &mem_data_, sizeof(state->mem_data) );

  state->cpu_count = cpu_count_;
  state->cpu_load = cpu_load_;
  state->cpu_self_avg = cpu_self_avg_;
  state->cpu_self_now = cpu_self_now_;
  if( cpu_count_ )
  {
    state->cpu_cores = new lh_cpudata[cpu_count_];
    memcpy( state->cpu_cores, &cpu_cores_curr_, sizeof(lh_cpudata) * cpu_count_ );
  }
  else
    state->cpu_cores = NULL;

  state->dir_binaries = filenameToUtf8( dir_binaries_ );
  state->dir_plugins = filenameToUtf8( dir_plugins_ );
  state->dir_data = filenameToUtf8( dir_data_ );
  state->dir_layout = filenameToUtf8( dir_layout_ );

  state->layout_file = filenameToUtf8( layout_file_ );
  state->layout_rps = layout_rps_;
}

void AppState::stateRefreshCPU( lh_systemstate* state )
{
  if( state->cpu_count != cpu_count_ )
  {
    if( state->cpu_cores ) delete[] state->cpu_cores;
    state->cpu_cores = new lh_cpudata[cpu_count_];
    state->cpu_count = cpu_count_;
  }
  memcpy( state->cpu_cores, &cpu_cores_curr_, sizeof(lh_cpudata) * cpu_count_ );

  state->cpu_load = cpu_load_;
  state->cpu_self_now = cpu_self_now_;
  state->cpu_self_avg = cpu_self_avg_;
  return;
}

void AppState::stateRefreshNetwork( lh_systemstate* state )
{
  state->net_max_in = net_max_in_;
  state->net_max_out = net_max_out_;
  state->net_cur_in = net_cur_in_;
  state->net_cur_out = net_cur_out_;
  memcpy( &state->net_data, &net_data_, sizeof(state->net_data) );
}

void AppState::stateRefreshMemory( lh_systemstate* state )
{
  memcpy( &state->mem_data, &mem_data_, sizeof(state->mem_data) );
}

void AppState::stateRefreshLayout( lh_systemstate* state )
{
  if( state->dir_layout ) delete[] state->dir_layout;
  state->dir_layout = filenameToUtf8( dir_layout_ );
  if( state->layout_file ) delete[] state->layout_file;
  state->layout_file = filenameToUtf8( layout_file_ );
}

void AppState::stateRefreshSecond( lh_systemstate* state )
{
  state->dev_fps = dev_fps_;
  state->run_time = run_time_;
  state->run_idle = 0;
  state->layout_rps = layout_rps_;
}

void AppState::stateTerm( lh_systemstate* state )
{
  if( state == NULL || state->size != sizeof(lh_systemstate) ) return;

  if( state->cpu_cores ) delete[] state->cpu_cores;
  if( state->dir_binaries ) delete[] state->dir_binaries;
  if( state->dir_plugins ) delete[] state->dir_plugins;
  if( state->dir_data ) delete[] state->dir_data;
  if( state->dir_layout ) delete[] state->dir_layout;
  if( state->layout_file ) delete[] state->layout_file;

  memset( state, 0, sizeof(lh_systemstate) );
}


static bool isValidAppObject_helper(QObject *parent, QObject *obj)
{
  if( parent == NULL ) return false;
  if( parent == obj ) return true;
  for( int i=0; i<parent->children().size(); i++ )
    if( isValidAppObject_helper(parent->children().at(i),obj) ) return true;
  return false;
}

bool AppState::isValidAppObject(QObject *obj)
{
  return isValidAppObject_helper( instance(), obj );
}
