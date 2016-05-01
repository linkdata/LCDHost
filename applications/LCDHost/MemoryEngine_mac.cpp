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

#include <QtDebug>

#include "MemoryEngine_mac.h"

#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/host_info.h>
#include <mach/vm_statistics.h>
#include <mach/task.h>
#include <mach/mach_host.h>

// http://web.mit.edu/darwin/src/modules/xnu/osfmk/man/

void MemoryEngine_mac::sample( lh_memdata *p_memdata ) const
{
    int mib[2];
    qint64 tot_phys = 0;
    qint64 used_phys = 0;
    size_t tot_phys_len;
    vm_statistics vmstat;
    vm_size_t pagesize;
    mach_msg_type_number_t vmstatsize = sizeof(vmstat);
    struct xsw_usage swapused; /* defined in sysctl.h */
    size_t swlen = sizeof(swapused);

    if( p_memdata == NULL ) return;

    if( host_page_size( mach_host_self(), &pagesize ) != KERN_SUCCESS ) return;

    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    tot_phys_len = sizeof(tot_phys);
    sysctl( mib, 2, &tot_phys, &tot_phys_len, NULL, 0 );

    if( host_statistics( mach_host_self(), HOST_VM_INFO, (integer_t*) (void*) &vmstat, &vmstatsize ) == KERN_SUCCESS )
    {
        used_phys += vmstat.active_count;
        used_phys += vmstat.inactive_count;
        used_phys += vmstat.wire_count;
        used_phys *= pagesize;
    }

    mib[0] = CTL_VM;
    mib[1] = VM_SWAPUSAGE;
    if( !sysctl( mib, 2, &swapused, &swlen, NULL, 0) )
    {
        p_memdata->tot_virt = swapused.xsu_total;
        p_memdata->free_virt = swapused.xsu_total - swapused.xsu_used;
    }

    p_memdata->tot_phys = tot_phys;
    p_memdata->free_phys = (tot_phys - used_phys);
}
