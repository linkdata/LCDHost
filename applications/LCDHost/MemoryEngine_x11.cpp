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

#include <string.h>
#include "MemoryEngine_x11.h"

MemoryEngine_x11::MemoryEngine_x11()
{
    procmeminfo = NULL;
}


MemoryEngine_x11::~MemoryEngine_x11()
{
    if( procmeminfo )
    {
        fclose( procmeminfo );
        procmeminfo = NULL;
    }
}

void MemoryEngine_x11::sample( lh_memdata *p )
{
    char buf[256];
    unsigned val;

    if( procmeminfo == NULL ) procmeminfo = fopen("/proc/meminfo","r");
    else fseek(procmeminfo, 0, SEEK_SET);
    if( procmeminfo == NULL ) return;

    while( !feof(procmeminfo) )
    {
        if( fgets(buf, sizeof(buf)-1, procmeminfo) == NULL) break;
        if( strncmp(buf, "MemTotal:", 9) == 0 )
        {
            sscanf(buf, "%*s %u", &val );
            if( p ) p->tot_phys = (qint64)val * Q_INT64_C(1024);
        }
        if( strncmp(buf, "MemFree:", 8) == 0 )
        {
            sscanf(buf, "%*s %u", &val );
            if( p ) p->free_phys = (qint64)val * Q_INT64_C(1024);
        }
        if( strncmp(buf, "SwapTotal:", 10) == 0 )
        {
            sscanf(buf, "%*s %u", &val );
            if( p ) p->tot_virt = (qint64)val * Q_INT64_C(1024);
        }
        if( strncmp(buf, "SwapFree:", 9) == 0 )
        {
            sscanf(buf, "%*s %u", &val );
            if( p ) p->free_virt = (qint64)val * Q_INT64_C(1024);
        }
    }
}

int MemoryEngine_x11::selfusage()
{
    return 0;
}
