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

#include <time.h>
#include <QDebug>
#include "CPUEngine_x11.h"

CPUEngine_x11::CPUEngine_x11()
    : procstat_(0)
{
    count_ = readProcStat();
}

CPUEngine_x11::~CPUEngine_x11()
{
    if (FILE* f = procstat_) {
        procstat_ = 0;
        fclose(f);
    }
}

qreal CPUEngine_x11::selfusage()
{
    return (qreal)clock() / (qreal)CLOCKS_PER_SEC / (qreal)count_;
}

int CPUEngine_x11::readProcStat(int count, lh_cpudata* data)
{
    int found = 0;

    if (procstat_ && fseek(procstat_, 0, SEEK_SET)) {
        fclose(procstat_);
        procstat_ = 0;
    }

    if (!procstat_)
        procstat_ = fopen("/proc/stat", "r");

    if (procstat_) {
        char buf[1024];
        unsigned cpu_user, cpu_nice, cpu_system, cpu_idle;
        qint64 when = QDateTime::currentDateTime().toMSecsSinceEpoch();
        while (fgets(buf, sizeof(buf) - 1, procstat_)) {
            if (strncmp(buf, "cpu", 3) == 0 && isdigit(buf[3])) {
                if (data && found < count) {
                    sscanf(buf, "%*s %u %u %u %u", &cpu_user, &cpu_nice, &cpu_system, &cpu_idle);
                    data[found].when = when;
                    data[found].idle = cpu_idle;
                    data[found].work = cpu_system + cpu_user + cpu_nice;
                }
                ++ found;
            }
        }
    }

    if (data && count > found)
        memset(data + found, 0, sizeof(lh_cpudata) * (count - found));

    return found;
}

void CPUEngine_x11::sample(int count, lh_cpudata* data)
{
    count_ = readProcStat(count, data);
    return;
}
