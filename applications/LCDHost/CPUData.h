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

#ifndef CPUDATA_H
#define CPUDATA_H

#include <QtGlobal>
#include <QTime>

class CPUCore
{
public:
    qint64 system;
    qint64 user;
    qint64 idle;
};

class CPUData
{
private:
    int count_;
    CPUCore *core_;
    QTime when_;

public:
    CPUData( int c )
    {
        count_ = 0;
        when_.start();
        core_ = new CPUCore[c];
        if( core_ ) count_ = c;
    }
    ~CPUData()
    {
        count_ = 0;
        if( core_ ) delete[] core_;
        core_ = NULL;
    }
    void setCore( int n, qint64 s, qint64 u, qint64 i )
    {
        if( n>= 0 && n<count_ )
        {
            core_[n].system = s;
            core_[n].user = u;
            core_[n].idle = i;
        }
        return;
    }
    int count() { return count_; }
    QTime when() { return when_; }
    int elapsed() { return when_.elapsed(); }
    const CPUCore *core(int n) { if( n>=0 && n<count_ ) return & core_[n]; return NULL; }
    qreal load(int n, CPUData *since = NULL );
};


#endif // CPUDATA_H
