/**
  \file     LH_QtPlugin_Graph.h
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  \legalese
    This module is based on original work by Johan Lindh.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

  */

#ifndef LH_QTPLUGIN_GRAPH_H
#define LH_QTPLUGIN_GRAPH_H

#include "LH_QtPlugin.h"
#include "DataCollection.h"
#include "LH_QtCPU.h"
#include "LH_QtNetwork.h"
#include <QTime>
#include <QDebug>

#define GRAPH_MEM_UNIT_BASE qreal(1024 * 1024 * 1024)

extern DataLineCollection* cpu_histogram_;
extern DataLineCollection* cpu_average_;
extern DataLineCollection* mem_physical_;
extern DataLineCollection* mem_virtual_;
extern DataLineCollection* net_in_;
extern DataLineCollection* net_out_;


class LH_QtPlugin_Graph : public LH_QtPlugin
{
    Q_OBJECT
    LH_QtCPU cpu_;
    LH_QtNetwork net_;
    QTime cpu_timer_;
    QTime mem_timer_;
    QTime net_timer_;

public:
    explicit LH_QtPlugin_Graph() : cpu_(this), net_(this) {
        cpu_histogram_ = NULL;
        cpu_average_ = NULL;
        mem_physical_ = NULL;
        mem_virtual_ = NULL;
        net_in_ = NULL;
        net_out_ = NULL;
    }

    const char *userInit();
    void userTerm();

    int notify(int n, void *p)
    {
        if (cpu_.count()!=0 && cpu_histogram_ != NULL)
        {
            if(cpu_histogram_->count()!=cpu_.count())
                cpu_histogram_->setCount(cpu_.count());
            if(cpu_average_->count()==0)
                cpu_average_->setCount(1);
            if(mem_physical_->count()==0)
                mem_physical_->setCount(1);
            if(mem_virtual_->count()==0)
                mem_virtual_->setCount(1);
            if(net_in_->count()==0)
                net_in_->setCount(1);
            if(net_out_->count()==0)
                net_out_->setCount(1);


            if(n&LH_NOTE_CPU)
            {
                int elapsed = cpu_timer_.elapsed();
                (*cpu_average_)[0].addValue(cpu_.averageload()/100, elapsed);
                for(int i =0; i<cpu_.count(); i++ )
                    (*cpu_histogram_)[i].addValue(cpu_.coreload(i)/100, elapsed);
                cpu_timer_.restart();
            }

            if(n&LH_NOTE_MEM)
            {
                int elapsed = mem_timer_.elapsed();
                (*mem_physical_)[0].addValue( (state()->mem_data.tot_phys - state()->mem_data.free_phys ) / GRAPH_MEM_UNIT_BASE , elapsed);
                (*mem_virtual_)[0].addValue( ( state()->mem_data.tot_virt - state()->mem_data.free_virt ) / GRAPH_MEM_UNIT_BASE , elapsed);
                mem_timer_.restart();
            }

            if(n&LH_NOTE_NET)
            {
                int elapsed = net_timer_.elapsed();
                (*net_in_)[0].addValue(net_.inRate(), elapsed);
                (*net_out_)[0].addValue(net_.outRate(), elapsed);
                net_timer_.restart();
            }

        }
        return LH_QtPlugin::notify(n,p) | cpu_.notify(n,p) | LH_NOTE_MEM | net_.notify(n,p);
    }

};

#endif // LH_QTPLUGIN_GRAPH_H
