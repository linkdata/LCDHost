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

#ifndef APPSTATE_H
#define APPSTATE_H

#include "lh_plugin.h"
#include "lh_systemstate.h"

#include "CPUEngine.h"
#include "MemoryEngine.h"
#include "NetworkEngine.h"

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QSize>
#include <QNetworkAccessManager>
#include <QAtomicPointer>

class AppState : public QObject
{
  Q_OBJECT
  static const int cpu_cores_max_ = 128;

  static QAtomicPointer<AppState> instance_;

  QSize dev_size_;
  int dev_depth_;
  int dev_fps_;
  int run_time_;
  int run_idle_;
  int net_max_in_;
  int net_max_out_;
  int net_cur_in_;
  int net_cur_out_;
  int cpu_count_;
  int cpu_load_;
  int cpu_self_now_;
  int cpu_self_avg_;
  lh_cpudata cpu_cores_curr_[cpu_cores_max_];
  lh_cpudata cpu_cores_prev_[cpu_cores_max_];
  qreal cpuLastSelfUsage_;
  qreal cpuLastSelfTime_;
  int layout_rps_;
  int fps_;
  int rps_;

  QString dir_binaries_;
  QString dir_plugins_;
  QString dir_data_;
  QString dir_layout_;
  QString layout_file_;

  CPUEngine_xxx cpuEngine_;
  MemoryEngine_xxx memEngine_;
  NetworkEngine_xxx netEngine_;

  lh_netdata net_data_;    /**< network load data */
  lh_netdata netLastSecond_;
  lh_memdata mem_data_;    /**< memory load data */

public:
  static bool isValidAppObject(QObject *obj);
  static AppState* instance();

  explicit AppState(QObject *parent = 0);
  ~AppState();

  char *filenameToUtf8( QString s );

  QString dir_binaries() const { return dir_binaries_; }
  QString dir_plugins() const { return dir_plugins_; }
  QString dir_data() const { return dir_data_; }
  QString dir_layout() const { return dir_layout_; }
  QString layout_file() const { return layout_file_; }

  void setLayout( QString path, QString file );

  CPUEngine_xxx& cpuEngine() { return cpuEngine_; }
  MemoryEngine_xxx& memEngine() { return memEngine_; }
  NetworkEngine_xxx& netEngine() { return netEngine_; }
  // RemoteWindow* remoteWindow() const { return remotewindow_; }

  void updateCPU();
  void updateNetwork( QString filter = QString() );
  void updateMemory();
  void updateSecond();

  int run_time() const { return run_time_; }

  lh_memdata& mem() { return mem_data_; }
  lh_netdata& net() { return net_data_; }

  int dev_fps() const { return dev_fps_; }
  void dev_fps_add() { ++fps_; }

  int cpu_load() const { return cpu_load_; }
  int cpu_self_now() const { return cpu_self_now_; }
  int cpu_self_avg() const { return cpu_self_avg_; }
  int net_cur_in() const { return net_cur_in_; }
  int net_cur_out() const { return net_cur_out_; }

  int layout_rps() const { return layout_rps_; }
  void layout_rps_add() { ++rps_; }

  void setNetMax( int in, int out ) { net_max_in_ = in; net_max_out_ = out; }

  void stateInit( lh_systemstate* );
  void stateRefreshCPU( lh_systemstate* );
  void stateRefreshNetwork( lh_systemstate* );
  void stateRefreshMemory( lh_systemstate* );
  void stateRefreshLayout( lh_systemstate* );
  void stateRefreshSecond( lh_systemstate* );
  void stateTerm( lh_systemstate* );
};

// #define APPSTATE AppState::instance()

int lh_cpuload(const lh_cpudata *from, const lh_cpudata *to , qint64 timeunit = -1); /* Returns average load between those times as permille */

#endif // APPSTATE_H
