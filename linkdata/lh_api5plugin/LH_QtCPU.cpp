/**
  \file     LH_QtCPU.cpp
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

#include "LH_QtCPU.h"
#include "lh_plugin.h"

LH_QtCPU::LH_QtCPU(LH_QtObject *parent)
    : parent_(parent)
{
    setup_smoothing_ = new LH_Qt_QSlider(parent_,"Smoothing",3,1,10);
    return;
}

LH_QtCPU::~LH_QtCPU()
{
    while( !load_.isEmpty() )
        delete[] load_.dequeue();
    return;
}

int LH_QtCPU::notify(int n, void *p)
{
    Q_UNUSED(p);

    if(n & LH_NOTE_CPU && parent_->state())
    {
        lh_cpudata *data;

        while( load_.size() >= samples() )
            delete[] load_.dequeue();

        data = new lh_cpudata[ state()->cpu_count ];
        if( data )
        {
            memcpy( data, state()->cpu_cores, sizeof(lh_cpudata) * state()->cpu_count );
            while( load_.size() >= samples() )
            {
                lh_cpudata *olddata = load_.dequeue();
                Q_ASSERT( olddata );
                if( olddata ) delete[] olddata;
            }
            load_.enqueue( data );
            if(!parent_->inherits(LH_QtPlugin::staticMetaObject.className()))
                parent_->requestRender();
        }
    }
    return LH_NOTE_CPU;
}

int LH_QtCPU::load(const lh_cpudata* from, const lh_cpudata* to)
{
    if (!from || !to ||
        !to->when || !from->when ||
        to->when == from->when ||
        (to->idle == from->idle && to->work == from->work))
        return 0;
    if (from->when > to->when)
        qSwap(from, to);
    qreal idle_delta = (qreal)(to->idle - from->idle);
    qreal work_delta = (qreal)(to->work - from->work);
    qreal load = 1.0 - idle_delta / (idle_delta + work_delta);
    Q_ASSERT(load >= 0.0 && load <= 1.0);
    return (int) (10000.0 * load);
}

int LH_QtCPU::coreload(int n)
{
    if( n<0 || n>=count() ) return 0;
    if( count() < 1 || load_.size() < 2 ) return 0;
    return load( & load_.first()[n], & load_.last()[n] );
}

int LH_QtCPU::averageload()
{
    int retv = 0;
    if( count() < 1 || load_.size() < 2 ) return 0;
    for( int i = 0; i<count(); i++ )
    {
        retv += load( & load_.first()[i], & load_.last()[i] );
    }
    retv /= count();
    Q_ASSERT( retv >= 0 );
    Q_ASSERT( retv <= 10000 );
    return retv;
}

