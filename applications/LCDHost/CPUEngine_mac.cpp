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

#include "CPUEngine_mac.h"

#ifdef Q_OS_MAC
# include <mach/boolean.h>
# include <mach/mach_host.h>
# include <mach/mach.h>
# include <stdarg.h>
# include <sys/time.h>
# include <sys/sysctl.h>
#endif

CPUEngine_mac::CPUEngine_mac()
{
    count_ = 1;
}

void CPUEngine_mac::sample( int count, lh_cpudata *data )
{
    unsigned int cpu_count;
    processor_cpu_load_info_t cpu_load;
    mach_msg_type_number_t cpu_msg_count;

    if( data == NULL || count<1 ) return;
    memset( data, 0, sizeof(lh_cpudata)*count );

    if( host_processor_info (mach_host_self (),
                             PROCESSOR_CPU_LOAD_INFO,
                             &cpu_count,
                             (processor_info_array_t *) & cpu_load,
                             &cpu_msg_count) == KERN_SUCCESS )
    {
        qint64 when = QDateTime::currentDateTime().toMSecsSinceEpoch();
        count_ = cpu_count;
        if (count > count_) {
            memset(data, 0, sizeof(lh_cpudata) * count);
            count = count_;
        }
        for(int n = 0; n < count_; ++n)
        {
            data[n].when = when;
            data[n].idle = cpu_load[n].cpu_ticks[CPU_STATE_IDLE];
            data[n].work = cpu_load[n].cpu_ticks[CPU_STATE_USER] +
                    cpu_load[n].cpu_ticks[CPU_STATE_NICE] +
                    cpu_load[n].cpu_ticks[CPU_STATE_SYSTEM]
                    ;
            /*
            data[n].user = cpu_load[n].cpu_ticks[CPU_STATE_USER];
            data[n].user += cpu_load[n].cpu_ticks[CPU_STATE_NICE];
            data[n].system = cpu_load[n].cpu_ticks[CPU_STATE_SYSTEM];
            data[n].total = cpu_load[n].cpu_ticks[CPU_STATE_IDLE] + data[n].system + data[n].user;
            Q_ASSERT( data[n].total >= (data[n].user+data[n].system) );
            */
        }

        vm_deallocate (mach_task_self (),
                       (vm_address_t) cpu_load,
                       (vm_size_t) (cpu_msg_count * sizeof (*cpu_load)));
    }

    return;
}

qreal CPUEngine_mac::selfusage()
{
    return (qreal)clock() / (qreal)CLOCKS_PER_SEC / (qreal)count_;
}
